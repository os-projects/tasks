from pprint import pprint
from random import randint
import re


MEMORY = list()
FLAGS = dict()
REGISTERS = dict()
DEAD_WIRES = list() # names of dead wires.


class Command(object):
	def __init__(self, kop, addr):
		self.kop_part = dict()
		self.kop_part['коп'] = kop
		op, ip = divmod(kop, 16)
		self.kop_part['оп'] = op

		if op != 0xF:
			i, p = divmod(ip, 4)
			self.kop_part['и'] = i
			self.kop_part['п'] = p
		else:
			self.kop_part['и'] = 0
			self.kop_part['п'] = 4

		self.addr_part = addr



def load_command_from_mem(addr):
	kop, quot, rem = MEMORY[addr:addr + 3]
	return Command(kop, quot * 256 + rem)


def get_mem(addr):
	return MEMORY[addr] * 256 + MEMORY[addr + 1]


def set_mem(addr, value):
	MEMORY[addr:addr + 2] = divmod(value, 256)


def set_flags(kop_part):
	kop = kop_part['коп']
	i = kop_part['и']
	p = kop_part['п']
	
	FLAGS['запп'] = (p == 0)
	FLAGS['зам1'] = (p == 1)
	FLAGS['зам2'] = (p != 3)
	FLAGS['взап1'] = (p == 3)
	FLAGS['выб'] = i
	FLAGS['чист'] = not(p == 2 or p == 3)
	FLAGS['пуск'] = (kop != 0xFF)

	priznak = REGISTERS['РОН']['признак'] if alive('прзнк') else 0
	if kop == 0xFE:
		FLAGS['перех'] = 1
	elif kop == 0xF0:
		FLAGS['перех'] = 1 - (priznak // 16)
	elif kop == 0xF1:
		FLAGS['перех'] = priznak % 16
	elif kop == 0xF4:
		FLAGS['перех'] = 1 - FLAGS['Ф']
	elif kop == 0xF5:
		FLAGS['перех'] = FLAGS['Ф']
	else:
		FLAGS['перех'] = 0

	for flag in FLAGS:
		FLAGS[flag] = int(FLAGS[flag]) if alive(flag) else 0



def normalize(val):  # value to 2-byte result
	rem = val % 2**16
	if rem >= 2 ** 15:
		rem -= 2 ** 16
	return rem


def form_alu_result(first_arg, op, second_arg):
	if op == 0:
		return normalize(second_arg)
	elif op == 1:
		return normalize(first_arg)
	elif op == 2:
		return normalize(first_arg + second_arg)
	elif op == 3:
		return normalize(second_arg - first_arg) # or vice versa?


def form_alu_pr(alu_result):
	return (alu_result != 0) * 16 + (alu_result <= 0)


def init_memory():
	global MEMORY
	file_content = open('program.txt').read()
	MEMORY = [int(byte, 16) for byte in re.findall('\w\w', file_content)]
	MEMORY.extend([0] * (2 ** 16))


def init_dead_wires():
	global DEAD_WIRES
	DEAD_WIRES = []


def init_flags():
	FLAGS['пуск'] = True
	FLAGS['Ф'] = False


def init_registers():
	REGISTERS['РОН'] = {'признак' : randint(0, 255), 'значение' : normalize(randint(1, 2 ** 16))}
	REGISTERS['ИР'] = normalize(randint(1, 2 ** 16))
	REGISTERS['УККОМ'] = 0


def print_machine_state():
	for i in range(0, 48, 16):
		print(' '.join(hex(byte)[2:].zfill(2) for byte in MEMORY[i:i + 16]))

	pprint(FLAGS)
	print()
	for key in REGISTERS:
		print("%s <- %s" % (key, REGISTERS[key]))
	print('\n')


def alive(*wire_names):
	return not any(name in DEAD_WIRES for name in wire_names)



if __name__ == '__main__':
	init_memory()
	init_dead_wires()
	init_flags()
	init_registers()
	EMPTY_COMMAND = Command(0, 0)

	while FLAGS['пуск']:
		com_ptr = REGISTERS['УККОМ'] if alive('адрком') else 0
		new_com_ptr = com_ptr + 3 if alive('адрком') else 0

		command = load_command_from_mem(com_ptr)
		if not alive('ком'):
			command = EMPTY_COMMAND

		if alive('коп'):
			set_flags(command.kop_part)
		else:
			set_flags(EMPTY_COMMAND.kop_part)

		ir_val = REGISTERS['ИР'] if alive('инд') else 0
		addr = command.addr_part if alive('а') else 0
		isp_addr = ir_val + addr if alive('иа') else 0

		first_arg = (get_mem(isp_addr), isp_addr)[FLAGS['выб']]
		second_arg = REGISTERS['РОН']['значение'] if alive('сум') else 0
		op = command.kop_part['оп'] if alive('оп') else 0
		alu_result = form_alu_result(first_arg, op, second_arg) if alive('рез1') else 0
		
		if FLAGS['зам1']:
			REGISTERS['РОН']['значение'] = alu_result
			REGISTERS['РОН']['признак'] = form_alu_pr(alu_result) if alive('пр') else 0

		val_to_ir = (alu_result, 0)[FLAGS['чист']]
		if FLAGS['зам2']:
			REGISTERS['ИР'] = val_to_ir

		REGISTERS['УККОМ'] = (new_com_ptr, isp_addr)[FLAGS['перех']]

	print_machine_state()

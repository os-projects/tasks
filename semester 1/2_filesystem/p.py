import random
import os
import sys


class AccessError(IOError):
	def __init__(self, argstring):
		super(AccessError, self).__init__(argstring)


def open_file(filename):
	failure_chance = 0.15
	if random.random() < failure_chance:
		raise IOError('Can\'t open file %s [mock]' % filename)
	if random.random() < failure_chance:
		raise AccessError('Can\'t get access to file %s [mock]' % filename)
	return open(filename)


def get_lines(fin):
	failure_chance = 0.05
	for i, line in enumerate(fin):
		try:
			if random.random() < failure_chance:
				raise IOError('Cant read line %d [mock]' % i)
			yield line.strip()
		except IOError as e:
			print(e)



def print_numbers(arr):
	arr = sorted(arr)
	chunk_len = 6
	chunks = [arr[i:i + chunk_len] for i in range(0, len(arr), chunk_len)]
	for chunk in chunks:
		pattern = "{:<10} " * len(chunk)
		print(pattern.format(*chunk))


def main():
	all_numbers = []
	root = '.\\files' if len(sys.argv) < 2 else sys.argv[1]
	for root, _, filenames in os.walk(root):
		for filename in filenames:
			filename = os.path.join(root, filename)
			try:
				fin = open_file(filename)
			except IOError as e:
				print(e)
			else:
				print('Opened %s' % filename)
				lines = get_lines(fin)
				for line in lines:
					for part in line.split():
						if part.isdigit():
							all_numbers.append(int(part))
	print_numbers(all_numbers)


if __name__ == '__main__':
	main()

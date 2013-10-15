import random
import os
import sys
from pprint import pprint


def open_file(filename):
	failure_chance = 0.15
	if random.random() < failure_chance:
		raise IOError('Can\'t open file %s [mock]' % filename)
	return open(filename)


def get_lines(fin):
	failure_chance = 0.05
	lines = []
	for i, line in enumerate(fin):
		if random.random() < failure_chance:
			print(IOError('Cant read line %d [mock]' % i))
		lines.append(line.strip())
	return lines


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
				lines = [int(line) for line in lines if line.isdigit()]
				all_numbers.extend(lines)
	pprint(sorted(all_numbers))

if __name__ == '__main__':
	main()

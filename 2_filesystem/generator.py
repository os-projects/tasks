import uuid
import sys
import os
from random import randrange, choice		


def randstr():
	return uuid.uuid4().hex[:8]


def fill_file_with_entries(fout):
	entries_count = randrange(50)
	for _ in range(entries_count):
		entry = randstr() if randrange(2) else str(randrange(2 ** 32))
		fout.write(entry + '\n')


def create_random_file(root):
	root = os.path.abspath(root)
	filename = randstr() + '.txt'
	fullpath = os.path.join(root, filename)
	if not os.path.exists(fullpath):
		fout = open(fullpath, 'w')
		fill_file_with_entries(fout)
		return fullpath


def create_random_directory(root):
	root = os.path.abspath(root)
	dirname = randstr()
	fullpath = os.path.join(root, dirname)
	if not os.path.exists(fullpath):
		os.makedirs(fullpath)
		return fullpath


def main():
	root = '.\\files' if len(sys.argv) < 2 else sys.argv[1]
	if not os.path.exists(root):
		os.makedirs(root)

	dirs_count = 4
	files_count = 20

	possible_dirs = [root]
	for _ in range(dirs_count):
		randdir = choice(possible_dirs)
		new_dirname = create_random_directory(randdir)
		if new_dirname is not None:
			possible_dirs.append(new_dirname)

	for _ in range(files_count):
		randdir = choice(possible_dirs)
		create_random_file(randdir)


if __name__ == '__main__':
	main()
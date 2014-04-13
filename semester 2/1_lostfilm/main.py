#!/usr/bin/env python
import sys
import Image
from collections import defaultdict


class ButtonFinder(object):
    def __init__(self, imagename):
        image = Image.open(imagename)
        self.width, self.height = image.size
        self.pixels = image.load()

    def get_red_point(self):
        answers = defaultdict(int)
        dx, dy = 15, 5
        for x in range(dx, self.width - dx):
            for y in range(dy, self.height - dy):
                answer = answers[x - 1, y]
                for yy in range(y - dy, y + dy):
                    answer -= self._is_red(x - dx, yy)
                    answer += self._is_red(x + dx, yy)
                answers[x, y] = answer
        return max(answers, key=answers.get)

    def _is_red(self, xx, yy):
        red, green, blue = self.pixels[xx, yy]
        return 200 < red and blue + green < 60

                

def main():
    imagename = sys.argv[1]
    bf = ButtonFinder(imagename)
    x, y = bf.get_red_point()
    print("{} {}".format(x, y))


if __name__ == '__main__':
    main()

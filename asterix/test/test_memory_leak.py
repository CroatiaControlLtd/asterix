# Test for memory leak problem in https://github.com/CroatiaControlLtd/asterix/issues/231

import asterix
from memory_profiler import profile

@profile
def func(data):
    p = asterix.parse_with_offset(data, 0, 100)
    #p = asterix.parse(data)
    #print(p)

if __name__ == '__main__':
    print(asterix.__version__)

    with open('../sample_data/cat062cat065.raw', 'rb') as f:
        data = f.read()

    for i in range(10):
        print('\n%d ------------------' % i)
        func(data)
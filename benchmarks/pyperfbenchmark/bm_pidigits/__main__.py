# coding: utf-8
"""
Calculating some of the digits of π.

This benchmark stresses big integer arithmetic.

Adapted from code on:
http://benchmarksgame.alioth.debian.org/
"""

import itertools

from six.moves import map as imap
import pyperf
import threading


DEFAULT_DIGITS = 2000
icount = itertools.count
islice = itertools.islice


def gen_x():
    return imap(lambda k: (k, 4 * k + 2, 0, 2 * k + 1), icount(1))


def compose(a, b):
    aq, ar, as_, at = a
    bq, br, bs, bt = b
    return (aq * bq,
            aq * br + ar * bt,
            as_ * bq + at * bs,
            as_ * br + at * bt)


def extract(z, j):
    q, r, s, t = z
    return (q * j + r) // (s * j + t)


def gen_pi_digits():
    z = (1, 0, 0, 1)
    x = gen_x()
    while 1:
        y = extract(z, 3)
        while y != extract(z, 4):
            z = compose(z, next(x))
            y = extract(z, 3)
        z = compose((10, -10 * y, 0, 1), z)
        yield y


def calc_ndigits(n):
    return list(islice(gen_pi_digits(), n))


def add_cmdline_args(cmd, args):
    cmd.extend(("--digits", str(args.digits)))


def functionWorker(runner, tid, digits):
    bmk_name = 'pidigits_' + str(tid)
    runner.bench_func(bmk_name, calc_ndigits, digits)

def main(params):
    nloops  = ('loops'   in params) and int(params['loops']) or 1
    workers = ('workers' in params) and int(params['workers']) or 1
    
    runner  = pyperf.Runner(add_cmdline_args=add_cmdline_args, loops=nloops)
    cmd = runner.argparser
    cmd.add_argument("--digits", type=int, default=DEFAULT_DIGITS,
                     help="Number of computed pi digits (default: %s)"
                          % DEFAULT_DIGITS)

    args = runner.parse_args()
    runner.metadata['description'] = "Compute digits of pi."
    runner.metadata['pidigits_ndigit'] = args.digits

    threads = []
    for i in range(workers):
        threads.append(threading.Thread(target=functionWorker, args=[runner,i, args.digits]))
    
    for idx, thread in enumerate(threads):
        thread.start()
        thread.join()
    
    out    =  'Executed '+str(workers)+' threads'
    result = {'output': out}

    return(result)
    
if __name__ == '__main__':
    main({'workers':1})

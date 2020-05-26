# coding: UTF-8
""" Telco Benchmark for measuring the performance of decimal calculations

- http://speleotrove.com/decimal/telco.html
- http://speleotrove.com/decimal/telcoSpec.html

A call type indicator, c, is set from the bottom (least significant) bit of the duration (hence c is 0 or 1).
A rate, r, is determined from the call type. Those calls with c=0 have a low r: 0.0013; the remainder (‘distance calls’) have a ‘premium’ r: 0.00894. (The rates are, very roughly, in Euros or dollarates per second.)
A price, p, for the call is then calculated (p=r*n). This is rounded to exactly 2 fractional digits using round-half-even (Banker’s round to nearest).
A basic tax, b, is calculated: b=p*0.0675 (6.75%). This is truncated to exactly 2 fractional digits (round-down), and the total basic tax variable is then incremented (sumB=sumB+b).
For distance calls: a distance tax, d, is calculated: d=p*0.0341 (3.41%). This is truncated to exactly 2 fractional digits (round-down), and then the total distance tax variable is incremented (sumD=sumD+d).
The total price, t, is calculated (t=p+b, and, if a distance call, t=t+d).
The total prices variable is incremented (sumT=sumT+t).
The total price, t, is converted to a string, s.

"""

from __future__ import print_function
from decimal import ROUND_HALF_EVEN, ROUND_DOWN, Decimal, getcontext, Context
import io
import os
from struct import unpack

import pyperf
import six
import threading
from six.moves import xrange


def rel_path(*path):
    return os.path.join(os.path.dirname("."), *path)


def bench_telco(loops, filename):
    getcontext().rounding = ROUND_DOWN
    rates = list(map(Decimal, ('0.0013', '0.00894')))
    twodig = Decimal('0.01')
    Banker = Context(rounding=ROUND_HALF_EVEN)
    basictax = Decimal("0.0675")
    disttax = Decimal("0.0341")

    with open(filename, "rb") as infil:
        data = infil.read()

    infil = io.BytesIO(data)
    outfil = six.StringIO()

    start = pyperf.perf_counter()
    for _ in range(loops):
        infil.seek(0)

        sumT = Decimal("0")   # sum of total prices
        sumB = Decimal("0")   # sum of basic tax
        sumD = Decimal("0")   # sum of 'distance' tax

        for i in xrange(5000):
            datum = infil.read(8)
            if datum == '':
                break
            n, =  unpack('>Q', datum)

            calltype = n & 1
            r = rates[calltype]

            p = Banker.quantize(r * n, twodig)

            b = p * basictax
            b = b.quantize(twodig)
            sumB += b

            t = p + b

            if calltype:
                d = p * disttax
                d = d.quantize(twodig)
                sumD += d
                t += d

            sumT += t
            print(t, file=outfil)

        outfil.seek(0)
        outfil.truncate()

    return pyperf.perf_counter() - start


def functionWorker(runner, tid):
    runner.metadata['description'] = "Telco decimal benchmark"

    filename = rel_path("data", "telco-bench.b")
    name = "telco_%s" % str(tid)
    runner.bench_time_func(name, bench_telco, filename)


def main(params):
    nloops  = ('loops'   in params) and int(params['loops']) or 1
    workers = ('workers' in params) and int(params['workers']) or 1
    
    runner  = pyperf.Runner(loops=nloops)

    threads = []
    for i in range(workers):
        threads.append(threading.Thread(target=functionWorker, args=[runner,i]))    
    
    for idx, thread in enumerate(threads):
        thread.start()
        thread.join()
    
    out    =  'Executed '+str(workers)+' threads'
    result = {'output': out}

    return(result)

if __name__ == "__main__":
    main({"workers":2})

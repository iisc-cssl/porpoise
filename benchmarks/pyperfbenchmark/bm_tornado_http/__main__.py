
"""Test the performance of simple HTTP serving and client using the Tornado
framework.

A trivial "application" is generated which generates a number of chunks of
data as a HTTP response's body.
"""

import sys
import socket

from six.moves import xrange
import pyperf
import threading

from tornado.httpclient import AsyncHTTPClient
from tornado.httpserver import HTTPServer
from tornado.gen import coroutine, Task
from tornado.ioloop import IOLoop
from tornado.netutil import bind_sockets
from tornado.web import RequestHandler, Application


HOST = "127.0.0.1"
FAMILY = socket.AF_INET

CHUNK = b"Hello world\n" * 1000
NCHUNKS = 5

CONCURRENCY = 150


class MainHandler(RequestHandler):

    @coroutine
    def get(self):
        for i in range(NCHUNKS):
            self.write(CHUNK)
            yield Task(self.flush)

    def compute_etag(self):
        # Overriden to avoid stressing hashlib in this benchmark
        return None


def make_application():
    return Application([
        (r"/", MainHandler),
    ])


def make_http_server(request_handler):
    server = HTTPServer(request_handler)
    sockets = bind_sockets(0, HOST, family=FAMILY)
    assert len(sockets) == 1
    server.add_sockets(sockets)
    sock = sockets[0]
    return server, sock


def bench_tornado(loops):
    server, sock = make_http_server(make_application())
    host, port = sock.getsockname()
    url = "http://%s:%s/" % (host, port)
    namespace = {}

    @coroutine
    def run_client():
        client = AsyncHTTPClient()
        range_it = xrange(loops)
        t0 = pyperf.perf_counter()

        for _ in range_it:
            futures = [client.fetch(url) for j in xrange(CONCURRENCY)]
            for fut in futures:
                resp = yield fut
                buf = resp.buffer
                buf.seek(0, 2)
                assert buf.tell() == len(CHUNK) * NCHUNKS

        namespace['dt'] = pyperf.perf_counter() - t0
        client.close()

    IOLoop.current().run_sync(run_client)
    server.stop()

    return namespace['dt']


def functionWorker (runner, tid):
    runner.metadata['description'] = ("Test the performance of HTTP requests "
                                      "with Tornado.")
    name = "tornado_http_%s" % str (tid)
    runner.bench_time_func(name, bench_tornado)


def main (params):
    # 3.8 changed the default event loop to ProactorEventLoop which doesn't
    # implement everything required by tornado and breaks this benchmark.
    # Restore the old WindowsSelectorEventLoop default for now.
    # https://bugs.python.org/issue37373
    # https://github.com/python/pyperformance/issues/61
    # https://github.com/tornadoweb/tornado/pull/2686
    nloops  = ('loops'   in params) and int(params['loops']) or 1
    workers = ('workers' in params) and int(params['workers']) or 1

    if sys.platform == 'win32' and sys.version_info[:2] == (3, 8):
        import asyncio
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    kw = {}
    kw['loops'] = nloops
    if pyperf.python_has_jit():
        # PyPy needs to compute more warmup values to warmup its JIT
        kw['warmups'] = 30
    runner  = pyperf.Runner(**kw)

    threads = []
    for i in range(workers):
        threads.append(threading.Thread(target=functionWorker, args=[runner,i]))    
    
    for idx, thread in enumerate(threads):
        thread.start()
        thread.join()
    
    out    =  'Executed '+str(workers)+' threads'
    result = {'output': out}
    return(result)


#if __name__ == "__main__":
#    main ({"workers":2})

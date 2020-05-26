"""
Benchmark for test the performance of Mako templates engine.
Includes:
    -two template inherences
    -HTML escaping, XML escaping, URL escaping, whitespace trimming
    -function defitions and calls
    -forloops
"""

import functools
import sys
import threading

import pyperf
from six.moves import xrange

# Mako imports (w/o markupsafe)
sys.modules['markupsafe'] = None

import mako   # noqa
from mako.template import Template   # noqa
from mako.lookup import TemplateLookup   # noqa


__author__ = "virhilo@gmail.com (Lukasz Fidosz)"

LOREM_IPSUM = """Quisque lobortis hendrerit posuere. Curabitur
aliquet consequat sapien molestie pretium. Nunc adipiscing luc
tus mi, viverra porttitor lorem vulputate et. Ut at purus sem,
sed tincidunt ante. Vestibulum ante ipsum primis in faucibus
orci luctus et ultrices posuere cubilia Curae; Praesent pulvinar
sodales justo at congue. Praesent aliquet facilisis nisl a
molestie. Sed tempus nisl ut augue eleifend tincidunt. Sed a
lacinia nulla. Cras tortor est, mollis et consequat at,
vulputate et orci. Nulla sollicitudin"""

BASE_TEMPLATE = """
<%def name="render_table(table)">
    <table>
    % for row in table:
        <tr>
        % for col in row:
            <td>${col|h}</td>
        % endfor
        </tr>
    % endfor
    </table>
</%def>
<%def name="img(src, alt)">
    <img src="${src|u}" alt="${alt}" />
</%def>
<html>
    <head><title>${title|h,trim}</title></head>
    <body>
        ${next.body()}
    </body>
<html>
"""

PAGE_TEMPLATE = """
<%inherit file="base.mako"/>
<table>
    % for row in table:
        <tr>
            % for col in row:
                <td>${col}</td>
            % endfor
        </tr>
    % endfor
</table>
% for nr in xrange(img_count):
    ${parent.img('/foo/bar/baz.png', 'no image :o')}
% endfor
${next.body()}
% for nr in paragraphs:
    <p>${lorem|x}</p>
% endfor
${parent.render_table(table)}
"""

CONTENT_TEMPLATE = """
<%inherit file="page.mako"/>
<%def name="fun1()">
    <span>fun1</span>
</%def>
<%def name="fun2()">
    <span>fun2</span>
</%def>
<%def name="fun3()">
    <span>foo3</span>
</%def>
<%def name="fun4()">
    <span>foo4</span>
</%def>
<%def name="fun5()">
    <span>foo5</span>
</%def>
<%def name="fun6()">
    <span>foo6</span>
</%def>
<p>Lorem ipsum dolor sit amet, consectetur adipiscing elit.
Nam laoreet justo in velit faucibus lobortis. Sed dictum sagittis
volutpat. Sed adipiscing vestibulum consequat. Nullam laoreet, ante
nec pretium varius, libero arcu porttitor orci, id cursus odio nibh
nec leo. Vestibulum dapibus pellentesque purus, sed bibendum tortor
laoreet id. Praesent quis sodales ipsum. Fusce ut ligula sed diam
pretium sagittis vel at ipsum. Nulla sagittis sem quam, et volutpat
velit. Fusce dapibus ligula quis lectus ultricies tempor. Pellente</p>
${fun1()}
${fun2()}
${fun3()}
${fun4()}
${fun5()}
${fun6()}
"""


def bench_mako(runner, table_size, nparagraph, img_count, tid):
    lookup = TemplateLookup()
    lookup.put_string('base.mako', BASE_TEMPLATE)
    lookup.put_string('page.mako', PAGE_TEMPLATE)

    template = Template(CONTENT_TEMPLATE, lookup=lookup)

    table = [xrange(table_size) for i in xrange(table_size)]
    paragraphs = xrange(nparagraph)
    title = 'Hello world!'

    func = functools.partial(template.render,
                             table=table, paragraphs=paragraphs,
                             lorem=LOREM_IPSUM, title=title,
                             img_count=img_count, xrange=xrange)

    bmk_name = 'mako_' + str(tid)
    runner.bench_func(bmk_name, func)


def functionWorker(runner, tid):
    table_size = 150
    nparagraph = 50
    img_count = 50
    bench_mako(runner, table_size, nparagraph, img_count, tid)

def main(params):
    nloops  = ('loops'   in params) and int(params['loops']) or 1
    workers = ('workers' in params) and int(params['workers']) or 1
    
    runner  = pyperf.Runner(loops=nloops)
    runner.metadata['description'] = "Mako templates"
    runner.metadata['mako_version'] = mako.__version__

    threads = []
    for i in range(workers):
        threads.append(threading.Thread(target=functionWorker, args=[runner,i]))    
    
    for idx, thread in enumerate(threads):
        thread.start()
        thread.join()
    
    out    =  'Executed '+str(workers)+' threads'
    result = {'output': out}

    return(result)

#if __name__ == '__main__':
#    main({'workers':2})

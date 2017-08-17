import argparse
import json
import pygal

parser = argparse.ArgumentParser(description='Solver performance graphs')
parser.add_argument('files', metavar='files', type=str, nargs='+',
                   help='json performance files')

args = parser.parse_args()

print('Loading files: ' + str(args.files))

json_benchmarks = {};

for file in args.files:
    with open(file) as json_data:
        json_benchmarks[file] = json.load(json_data)

benchmark_chart = pygal.Bar(range=(0.0, 600.0))
names = []
for benchmark in json_benchmarks.values()[0]['benchmarks']:
    names.append(benchmark['name'])

benchmark_chart.x_labels = tuple(names)

times = {}
for name, json_benchmark in json_benchmarks.iteritems():
    for benchmark in json_benchmark['benchmarks']:
        times.setdefault(name, []).append(benchmark['real_time'])

for name, time in times.iteritems():
    benchmark_chart.add(name, time)

benchmark_chart.render_to_file("benchmarks.svg")

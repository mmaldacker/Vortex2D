import argparse
import json
import pygal

parser = argparse.ArgumentParser(description='CG Breakdown')
parser.add_argument('file', metavar='file', type=str,
                   help='breakdown file')

args = parser.parse_args()

breakdown_chart = pygal.Pie()
breakdown_chart.title = 'CG breakdown'

file = open(args.file, 'r')
for line in file.readlines():
    split_line = line.split(":")
    breakdown_chart.add(split_line[0], float(split_line[1].strip()))

breakdown_chart.render_to_file("cg_breakdown.svg")

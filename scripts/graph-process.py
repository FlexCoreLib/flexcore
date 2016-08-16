#!/usr/bin/env python3
import sys
import re
import collections
import argparse
import json

parser = argparse.ArgumentParser(description='Merge connection graph and forest data.')
parser.add_argument('forest', type=str, help='json file containing forest')
parser.add_argument('graph', type=str, help='graphviz dot file containing graph')
args = parser.parse_args()

counter = 0

class CompoundNode:
    def __init__(self, name=''):
        self.nodes = set()
        self.name = name

    def print(self,indent):
       global counter
       my_cnt = counter
       counter += 1
       out = ''
       out += '  ' * indent + 'subgraph cluster_%d {\n' % my_cnt
       out += '  ' * (indent+1) + 'label="%s";\n' % self.name
       for node in self.nodes:
           out += nodes[node].print(indent+1)
       if not self.nodes:
            out += '%d[label="%s"]\n' % (counter + len(lines), self.name)
       out += '  ' * indent + '}\n'
       return out

    def add(self, node):
        self.nodes.add(node)

    @staticmethod
    def from_node(node):
        compound_node = CompoundNode()
        compound_node.add(node)
        return compound_node


class Node:

    def __init__(self, num):
        self.num = num

    def print(self,indent):
        return '  ' * indent + '%s;\n' % self.num

def region_to_color(region):
    reg_int = int(region, 16)
    rgb = reg_int & 0xffffff
    return '#%x' % rgb

def line_to_node(line):
    match = re.match(r'(\d+)\[label="([^"]+)"', line)
    num = match.group(1)
    label = match.group(2)
    uuid = re.search(r'uuid="([^"]+)"', line).group(1)
    region = re.search(r'region="([^"]+)"', line).group(1)
    return num, label, uuid, region


with open(args.graph) as file_:
    graph_lines = file_.readlines()

root_nodes = collections.defaultdict(CompoundNode)
nodes = collections.defaultdict(CompoundNode)

lines = []
for line in graph_lines[1:]:
    if not re.match(r'\d+\[', line):
        break
    num, name, uuid, region = line_to_node(line)
    nodes[uuid] = Node(num)
    lines.append('%s[label="%s", fillcolor="%s", style="filled"];' % (num, name,
                 region_to_color(region)))


with open(args.forest) as file_:
    forest = json.load(file_, object_pairs_hook=collections.OrderedDict)

for node,parents in forest.items():
    # first entry in parents array is name of node
    name, *parents = parents
    if len(parents) == 0:
        root_nodes[node] = nodes[node]
    else:
        first_parent = parents[0]
        try:
            nodes[first_parent].add(node)
        except AttributeError:
            # HACK - subgraphs can't have connections so add the _old_ node as
            # a subnode to the new compound node
            nodes[first_parent + 'x'] = nodes[first_parent]
            nodes[first_parent] = CompoundNode.from_node(first_parent + 'x')
            nodes[first_parent].add(node)
    nodes[node].name = name

print(graph_lines[0],end='')
for root_node in root_nodes.values():
    print(root_node.print(0))
for line in lines:
    print(line)
for line in graph_lines[len(lines)+1:]:
    print(line,end='')


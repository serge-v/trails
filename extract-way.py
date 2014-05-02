#!/usr/bin/python

import xml.sax, sys

class way:
    nodes = []

class ContentHandler:
    stack = []
    ways = []
    path = ''
    nodes = {}
    name = ''
    way_nodes = []
    out = False
    lines = ''

    def startDocument(self):
        pass

    def endDocument(self):
        pass

    def startElement(self, name, attrs):
        self.stack.append(name)
        self.path = '/' + '/'.join(self.stack)

        if self.path == '/osm/way':
            self.lines = ''
            self.out = False
        elif self.path == '/osm/way/nd':
            ref = attrs.getValueByQName('ref')
            n = self.nodes[ref]
            self.lines += '    node %s %s\n' % (n[1], n[0])
        elif self.path == '/osm/way/tag':
            key = attrs.getValueByQName('k')
            val = attrs.getValueByQName('v')
            if key ==  'name':
                self.lines += '    name %s\n' % val
                self.name = val
            elif key ==  'highway':
                self.lines += '    highway %s\n' % val
            if val.find('hiki') >= 0 or val.find('trail') >= 0 or val.find('foot') >= 0:
                self.out = True
        elif self.path == '/osm/node':
            id = attrs.getValueByQName('id')
            lat = attrs.getValueByQName('lat')
            lon = attrs.getValueByQName('lon')
            self.nodes[id] = (lat, lon)

        depth = len(self.stack) - 1
        
#        print ''.ljust(depth * 4) + name
#        for a in attrs.getNames():
#            print ''.ljust(depth * 4) + '    ' + a + '=' + attrs.getValue(a)
#        print self.path

    def endElement(self, name):
        if self.path == '/osm/way':
            if self.out:
                print 'way red ' + self.name.lower().replace(' ', '-') + '\n' + self.lines
                self.out = False
            else:
                print >> sys.stderr, self.name + ' - skipped'
            self.lines = ''

        self.stack.pop()
        self.path = '/' + '/'.join(self.stack)

    def startElementNS(self, name, qname, attrs):
        depth = len(self.stack) - 1
        print ''.ljust(depth * 4) + str(attrs)
        pass

    def endElementNS(self, name, qname):
        pass

    def characters(self, content):
        pass

    def processingInstruction(self, target, data):
        pass

    def ignorableWhitespace(self, whitespace):
        pass

    def skippedEntity(self, name):
        print name

    def startPrefixMapping(self, prefix, uri):
        pass

    def endPrefixMapping(self, prefix):
        pass

    def setDocumentLocator(self, locator):
        pass

handler = ContentHandler()

xml.sax.parse('map.osm', handler)

#
#	LiAR isn't a raytracer
#	Copyright (C) 2004-2005  Bram de Greve (bramz@sourceforge.net)
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#	http://liar.sourceforge.net
#

import liar.scenery

def load(iFilename):
	class Element:
		def __init__(self, iName, iNumber):
			self.name = iName
			self.number = iNumber
			self.properties = []
			
	fieldConvertors = {
		'int8': int,
		'uint8': int,
		'int16': int,
		'uint16': int,
		'int32': int,
		'uint32': int,
		'float32': float,
		'float64': float,
		'char': int, 
		'uchar': int, 
		'short': int, 
		'ushort': int, 
		'int': int, 
		'uint': int, 
		'float': float, 
		'double': float
	}
			
	def loadHeader(iFile):
		if iFile.readline() != 'ply\n':
			raise "%s is not a PLY file" % iFilename
		if iFile.readline() != 'format ascii 1.0\n':
			raise "PLY file %s is not of a compatible format"
		
		elements = []
		currentElement = None
		
		while True:
			line = iFile.readline()
			assert(len(line) > 0)
			
			splittedLine = line.split()
			command, fields = splittedLine[0], splittedLine[1:]
			numFields = len(fields)
			
			if command == 'end_header':
				assert(numFields == 0)
				return elements
			
			if command == 'element':
				assert(numFields == 2)
				name = fields[0]
				number = int(fields[1])
				currentElement = Element(name, number)
				elements.append(currentElement)
				
			elif command == 'property':
				assert(numFields > 0)
				propType = fields[0]
				if propType == 'list':
					assert(numFields == 4)
					numType = fields[1]
					listType = fields[2]
					propName = fields[3]
					currentElement.properties.append({'type': propType, 'name': propName, 'numType': numType, 'listType': listType})
				else:
					assert(numFields == 2)
					propName = fields[1]
					currentElement.properties.append({'type': propType, 'name': propName})
			
			elif command == 'comment':
				pass
			
			else:				
				assert(False) # you should never be here
				
	def loadData(iFile, iElements):
		plyData = {}
		for element in iElements:
			elementData = []
			for e in xrange(element.number):
				line = iFile.readline()
				assert(len(line) > 0)
				
				lineData = []
				fields = line.split()
				numFields = len(fields)
				f = 0
				for prop in element.properties:
					assert(len(prop) > 0)
					propType = prop['type']
					if propType == 'list':
						assert(len(prop) == 4)
						numType = prop['numType']
						listType = prop['listType']
						assert(f < numFields)
						listLength = fieldConvertors[numType](fields[f])
						f += 1
						listData = []
						for k in xrange(listLength):
							assert(f < numFields)
							listData.append(fieldConvertors[listType](fields[f]))
							f += 1
						lineData.append(listData)
					else:
						assert(len(prop) == 2)
						assert(f < numFields)
						lineData.append(fieldConvertors[propType](fields[f]))
						f += 1
				assert(f == numFields)
						
				elementData.append(lineData)
				
			plyData[element.name] = elementData
		
		return plyData
				
	print "loading PLY file %s ..." % iFilename

	ply = file(iFilename)
	elements = loadHeader(ply)
	
	elementDict = dict([(element.name, element) for element in elements])
	
	# see about vertices ...
	assert('vertex' in elementDict)
	vertexElement = elementDict['vertex']
	vertexPropDict = dict([(prop['name'], pos) for prop, pos in zip(vertexElement.properties, range(len(vertexElement.properties)))])
	assert('x' in vertexPropDict and 'y' in vertexPropDict and 'z' in vertexPropDict)
	vertexX = vertexPropDict['x']
	vertexY = vertexPropDict['y']
	vertexZ = vertexPropDict['z']
	
	# see about faces
	assert('face' in elementDict)
	faceElement = elementDict['face']
	facePropDict = dict([(prop['name'], pos) for prop, pos in zip(faceElement.properties, range(len(faceElement.properties)))])
	assert('vertex_indices' in facePropDict)
	faceVertices = facePropDict['vertex_indices']
	
	data = loadData(ply, elements)
	
	vertices = []
	for v in data['vertex']:
		vertices.append((v[vertexX], v[vertexY], v[vertexZ]))
		
	triangles = []
	for face in data['face']:
		indices = face[faceVertices]
		assert(len(indices) >= 3)
		for i in range(1, len(indices) - 1):
			triangles.append(((indices[0],), (indices[i],), (indices[i+1],)))
		
	aabb_min = tuple([min([v[k] for v in vertices]) for k in range(3)])
	aabb_max = tuple([max([v[k] for v in vertices]) for k in range(3)])
	print '%s vertices, %s triangles, aabb [%s, %s]' % (len(vertices), len(triangles), aabb_min, aabb_max)
	
	return liar.scenery.TriangleMesh(vertices, [], [], triangles)

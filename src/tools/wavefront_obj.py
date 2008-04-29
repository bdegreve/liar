# LiAR isn't a raytracer
# Copyright (C) 2004-2007  Bram de Greve (bramz@users.sourceforge.net)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# http://liar.sourceforge.net

import liar.scenery
import liar.shaders
import liar.textures
import math

def load(filename):
	return decode(file(filename), filename)

def decode(lines, filename = ""):
	vertices = []
	normals = []
	uvs = []
	vectors = [vertices, normals, uvs]
	faces = []
	numFaces = 0
	groups = []
	materials = {'': liar.shaders.Lambert(liar.textures.Constant(liar.rgb(1, 1, 1)))}
	currentMaterial = materials['']
	
	def _int_or_none(x):
		try:
			return int(x)
		except:
			return None
	
	def _floatTuple(iFields):
		return tuple([float(x) for x in iFields])
	
	def _isZero(iTuple):
		return min([x == 0 for x in iTuple])
		
	def _normalize(n):
		norm = math.sqrt(sum([x * x for x in n]))
		if norm == 0:
			return n
		return tuple([x / norm for x in n])
		
	def _loadMaterials(filename):
		currentMaterial = None
		materials = {}
		for line in file(filename):
			def _enforceFields(iSuccess):
				if not iSuccess:
					raise "material libary %s has syntax error: %s" % (filename, line)
				
			fields = line.split()
			if len(fields) > 0:
				command, fields = fields[0], fields[1:]
				if command == 'newmtl':
					_enforceFields(len(fields) < 2)
					try:
						name = fields[0]
					except:
						name = ''
					currentMaterial = liar.shaders.Simple()
					materials[name] = currentMaterial
				elif command == 'Kd':
					_enforceFields(len(fields) == 3)
					currentMaterial.diffuse = liar.textures.Constant(liar.rgb(_floatTuple(fields)))
				elif command == 'Ks':
					_enforceFields(len(fields) == 3)
					currentMaterial.specular = liar.textures.Constant(liar.rgb(_floatTuple(fields)))
				elif command == 'Ka':
					_enforceFields(len(fields) == 3)
					pass
				elif command == 'Ns':
					_enforceFields(len(fields) == 1)
					currentMaterial.specularPower = liar.textures.Constant(float(fields[0]))
				elif command == 'map_Kd':
					_enforceFields(len(fields) == 1)
					currentMaterial.diffuse = liar.textures.Image(fields[0])
				elif command == 'map_Ks':
					_enforceFields(len(fields) == 1)
					currentMaterial.specular = liar.textures.Image(fields[0])
				elif command == 'map_Ka':
					_enforceFields(len(fields) == 1)
					pass
				elif command == 'map_Ns':
					_enforceFields(len(fields) == 1)
					currentMaterial.specularPower = liar.textures.Image(fields[0])
		return materials
	
	def _makeGroup(vertices, normals, uvs, faces, material):
		components = (vertices, normals, uvs)
		
		# compress vertices, normals and uvs to only those that are used by faces
		#
		# see what vertices are used, and assign them a new index.
		used_components = [{}, {}, {}]
		for face in faces:
			for vertex in face:
				for i, UCs in zip(vertex, used_components):
					if i != None and not i in UCs:
						UCs[i] = len(UCs)
		
		# build new (compressed) lists of vertices, uvs and normals
		sorted_components = [sorted(zip(UCs.values(), UCs.keys())) for UCs in used_components]
		compressed_components = [[Cs[j] for i, j in SCs] for Cs, SCs in zip(components, sorted_components)]
						
		# replace old indices by new ones
		for UCs in used_components:
			UCs[None] = None
		faces = [[[UCs[i] for i, UCs in zip(vertex, used_components)] for vertex in face] for face in faces]
		
		group = liar.scenery.TriangleMesh(compressed_components[0], compressed_components[1], compressed_components[2], faces)
		group.shader = material
		return group
			
	print "loading Wavefront OBJ file %s ..." % filename
	
	for line in lines:
		fields = line.split()
		if len(fields) > 0:
			
			def _enforceFields(iSuccess):
				if not iSuccess:
					raise "obj file %s has syntax error: %s" % (filename, line)
			
			command = fields[0]
			fields = fields[1:]
			if command == '#':
				pass
			
			if command == 'mtllib':
				_enforceFields(len(fields) > 0)
				for mtl in fields:
					materials.update(_loadMaterials(mtl))
			
			elif command == 'usemtl':
				_enforceFields(len(fields) < 2)
				try:
					name = fields[0]
				except:
					name = ''
				if name not in materials.keys():
					print "obj file %s uses unknown material '%s', using default instead: %s" % (filename, name, line)
					name = ''
				currentMaterial = materials[name]
			
			elif command == 'g':
				if len(faces) > 0:
					numFaces += len(faces)
					groups.append(_makeGroup(vertices, normals, uvs, faces, currentMaterial))
					faces = []
			
			elif command == 'v':
				_enforceFields(len(fields) == 3)
				vertices.append(_floatTuple(fields))
			
			elif command == 'vn':
				_enforceFields(len(fields) == 3)
				normals.append(_normalize(_floatTuple(fields)))
			
			elif command == 'vt':
				_enforceFields(len(fields) == 2)
				uvs.append(_floatTuple(fields))
			
			elif command == 'f':
				_enforceFields(len(fields) >= 3)
				face = [[_int_or_none(x) for x in field.split('/')] for field in fields]
				
				# make sure all indices are in correct range
				for vertex in face:
					length = len(vertex)
					for k in range(length):
						if vertex[k] != None:
							if vertex[k] > 0:
								vertex[k] -= 1
							elif vertex[k] < 0:
								vertex[k] += len(vectors[k])
							else:
								raise "obj file %s has 0 index on line: %s" % (filename, line)
				
				# reformat vertex data to (v, vn, vt) and make sure its of length 3 
				# (use None to fill missing values)
				size = len(face)
				numNormals = 0
				numUvs = 0
				for i in xrange(size):
					vertex = face[i]
					v = vertex[0]
					vt = None
					if len(vertex) > 1:
						vt = vertex[1]
					if vt != None:
						numUvs += 1
					vn = None
					if len(vertex) > 2:
						vn = vertex[2]
					if vn != None and _isZero(normals[vn]):
						vn = None
					if vn != None:
						numNormals += 1
					face[i] = (v, vn, vt)
					
				# make sure either none or all vertices have a normal/uv
				if numNormals > 0 and numNormals < size:
					#print "WARNING: either none or all face vertices must have normal, forcing to none"
					face = [(v[0], None, v[2]) for v in face]
				if numUvs > 0 and numUvs < size:
					#print "WARNING: either none or all face vertices must have uv coord, forcing to none"
					face = [(v[0], v[1], None) for v in face]				
					
				# fan-triangulate and add.
				for i in xrange(1, len(face) - 1):
					faces.append((face[0], face[i], face[i + 1]))
	
	if len(faces) > 0:
		numFaces += len(faces)
		groups.append(_makeGroup(vertices, normals, uvs, faces, currentMaterial))
		faces = []
	
	print "%s vertices, %s normals, %s uvs, %s faces" % (len(vertices), len(normals), len(uvs), numFaces)
	
	return groups

# EOF

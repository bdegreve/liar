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
import liar.shaders
import liar.textures

def load(iFilename):
	vertices = []
	normals = []
	uvs = []
	vectors = [vertices, normals, uvs]
	faces = []
	numFaces = 0
	groups = []
	materials = {'': liar.shaders.Simple(liar.textures.Constant(liar.rgb(1, 1, 1)))}
	currentMaterial = materials['']
	
	def _floatTuple(iFields):
		return tuple([float(x) for x in iFields])
	
	def _isZero(iTuple):
		return min([x == 0 for x in iTuple])
	
	def _loadMaterials(iFilename):
		currentMaterial = None
		materials = {}
		for line in file(iFilename):
			def _enforceFields(iSuccess):
				if not iSuccess:
					raise "material libary %s has syntax error: %s" % (iFilename, line)
				
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
		# compress vertices, normals and uvs to only those that are used by faces
		#
		# see what vertices are used, and assign them a new index.
		used_components = [{}, {}, {}]
		for face in faces:
			for vertex in face:
				for k in range(len(vertex)):
					i = vertex[k]
					if not i in used_components[k]:
						used_components[k][i] = len(used_components[k])
						
		# replace old indices by new ones
		faces = [[tuple([used_components[k][vertex[k]] for k in range(len(vertex))]) for vertex in face] for face in faces]
		
		# build new (compressed) lists of vertices, uvs and normals
		sorted_components = [[(used_comps[i], i) for i in used_comps] for used_comps in used_components]
		for k in range(3):
			sorted_components[k].sort()	
		compressed_uvs = [uvs[x[1]] for x in sorted_components[1]]
		compressed_vertices = [vertices[x[1]] for x in sorted_components[0]]
		compressed_normals = [normals[x[1]] for x in sorted_components[2]]
		
		group = liar.scenery.TriangleMesh(compressed_vertices, compressed_normals, compressed_uvs, faces)
		group.shader = material
		return group
			
	print "loading Wavefront OBJ file %s ..." % iFilename
	
	for line in file(iFilename):
		fields = line.split()
		if len(fields) > 0:
			
			def _enforceFields(iSuccess):
				if not iSuccess:
					raise "obj file %s has syntax error: %s" % (iFilename, line)
			
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
					print "obj file %s uses unknown material '%s', using default instead: %s" % (iFilename, name, line)
					name = ''
				currentMaterial = materials[name]
			
			elif command == 'g':
				if len(faces) > 0:
					numFaces += len(faces)
					groups.append(_makeGroup(vertices, uvs, normals, faces, currentMaterial))
					faces = []
			
			elif command == 'v':
				_enforceFields(len(fields) == 3)
				vertices.append(_floatTuple(fields))
			
			elif command == 'vn':
				_enforceFields(len(fields) == 3)
				normals.append(_floatTuple(fields))
			
			elif command == 'vt':
				_enforceFields(len(fields) == 2)
				uvs.append(_floatTuple(fields))
			
			elif command == 'f':
				_enforceFields(len(fields) >= 3)
				face = [[int(x) for x in field.split('/')] for field in fields]
				# make sure all indices are in correct range
				for vertex in face:
					length = len(vertex)
					for k in range(length):
						if vertex[k] > 0:
							vertex[k] -= 1
						elif vertex[k] < 0:
							vertex[k] += len(vectors[k])
						else:
							raise "obj file %s has 0 index on line: %s" % (iFilename, line)
				# reformat vertex data to (v, vn, vt) and make sure its of length 3 
				# (use None to fill missing values)
				for vertex in face:
					v = vertex[0]
					vt = None
					if len(vertex) > 1:
						vt = vertex[1]
					vn = None
					if len(vertex) > 2:
						vn = vertex[2]
					if vn and _isZero(normals[vn]):
						print "Warning: zero normal detected.  Disabling normal for this vertex"
						vn = None
					vertex = (v, vn, vt)
				# fan-triangulate and add.
				for i in xrange(1, len(face) - 1):
					faces.append((face[0], face[i], face[i + 1]))
	
	if len(faces) > 0:
		numFaces += len(faces)
		groups.append(_makeGroup(vertices, normals, uvs, faces, currentMaterial))
		faces = []
	
	print "%s vertices, %s normals, %s uvs, %s faces" % (len(vertices), len(normals), len(uvs), numFaces)
	
	return groups
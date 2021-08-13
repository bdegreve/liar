# LiAR isn't a raytracer
# Copyright (C) 2004-2021  Bram de Greve (bramz@users.sourceforge.net)
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
# http://liar.bramz.net/

import liar.scenery
from dataclasses import dataclass, field
from typing import List, Optional


@dataclass
class Property:
    type_: str
    name: str
    numType: Optional[str] = None
    listType: Optional[str] = None


@dataclass
class Element:
    name: str
    number: int
    properties: List[Property] = field(default_factory=list)


def load(path):
    fieldConvertors = {
        "int8": int,
        "uint8": int,
        "int16": int,
        "uint16": int,
        "int32": int,
        "uint32": int,
        "float32": float,
        "float64": float,
        "char": int,
        "uchar": int,
        "short": int,
        "ushort": int,
        "int": int,
        "uint": int,
        "float": float,
        "double": float,
    }

    def loadHeader(fp):
        if fp.readline() != "ply\n":
            raise ValueError(f"{path} is not a PLY file")
        if fp.readline() != "format ascii 1.0\n":
            raise ValueError(f"PLY file {path} is not of a compatible format")

        elements = []
        currentElement = None

        while True:
            line = fp.readline()
            assert len(line) > 0

            splittedLine = line.split()
            command, fields = splittedLine[0], splittedLine[1:]
            numFields = len(fields)

            if command == "end_header":
                assert numFields == 0
                return elements

            if command == "element":
                assert numFields == 2
                name = fields[0]
                number = int(fields[1])
                currentElement = Element(name, number)
                elements.append(currentElement)

            elif command == "property":
                assert numFields > 0
                propType = fields[0]
                if propType == "list":
                    assert numFields == 4
                    prop = Property(
                        propType, name=fields[3], numType=fields[1], listType=fields[2]
                    )
                else:
                    assert numFields == 2
                    prop = Property(propType, name=fields[1])
                currentElement.properties.append(prop)

            elif command == "comment":
                pass

            else:
                assert False  # you should never be here

    def loadData(fp, elements):
        plyData = {}
        for element in elements:
            elementData = []
            for e in range(element.number):
                line = fp.readline()
                assert len(line) > 0

                lineData = []
                fields = line.split()
                numFields = len(fields)
                f = 0
                for prop in element.properties:
                    if prop.type_ == "list":
                        assert f < numFields
                        listLength = fieldConvertors[prop.numType](fields[f])
                        f += 1
                        listData = []
                        for k in range(listLength):
                            assert f < numFields
                            listData.append(fieldConvertors[prop.listType](fields[f]))
                            f += 1
                        lineData.append(listData)
                    else:
                        assert prop.numType is None
                        assert prop.listType is None
                        assert f < numFields
                        lineData.append(fieldConvertors[prop.type_](fields[f]))
                        f += 1
                assert f == numFields

                elementData.append(lineData)

            plyData[element.name] = elementData

        return plyData

    print(f"loading PLY file {path} ...")

    with open(path) as fp:
        elements = loadHeader(fp)
        data = loadData(fp, elements)

    elementDict = {element.name: element for element in elements}

    # see about vertices ...
    assert "vertex" in elementDict
    vertexElement = elementDict["vertex"]
    vertexPropDict = {
        prop.name: pos for pos, prop in enumerate(vertexElement.properties)
    }
    assert "x" in vertexPropDict and "y" in vertexPropDict and "z" in vertexPropDict
    vertexX = vertexPropDict["x"]
    vertexY = vertexPropDict["y"]
    vertexZ = vertexPropDict["z"]

    # see about faces
    assert "face" in elementDict
    faceElement = elementDict["face"]
    facePropDict = {prop.name: pos for pos, prop in enumerate(faceElement.properties)}
    assert "vertex_indices" in facePropDict
    faceVertices = facePropDict["vertex_indices"]

    vertices = []
    for v in data["vertex"]:
        vertices.append((v[vertexX], v[vertexY], v[vertexZ]))

    triangles = []
    for face in data["face"]:
        indices = face[faceVertices]
        assert len(indices) >= 3
        for i in range(1, len(indices) - 1):
            triangles.append(((indices[0],), (indices[i],), (indices[i + 1],)))

    aabb_min = tuple([min([v[k] for v in vertices]) for k in range(3)])
    aabb_max = tuple([max([v[k] for v in vertices]) for k in range(3)])
    print(
        f"{len(vertices)} vertices, {len(triangles)} triangles, aabb [{aabb_min}, {aabb_max}]"
    )

    return liar.scenery.TriangleMesh(vertices, [], [], triangles)


# EOF

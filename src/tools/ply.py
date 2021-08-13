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

import enum
import io
import struct
from dataclasses import dataclass, field
from typing import List, Optional

import liar.scenery


@dataclass
class Property:
    type_: str
    name: str
    num_type: Optional[str] = None
    list_type: Optional[str] = None


@dataclass
class Element:
    name: str
    number: int
    properties: List[Property] = field(default_factory=list)


class Format(enum.Enum):
    ASCII_1_0 = "format ascii 1.0"
    BINARY_LITTLE_ENDIAN_1_0 = "format binary_little_endian 1.0"
    BINARY_BIG_ENDIAN_1_0 = "format binary_big_endian 1.0"


@dataclass
class Header:
    format: Format
    elements: List[Element]
    dataOffset: int


def load(path: str) -> liar.scenery.TriangleMesh:

    print(f"loading PLY file {path} ...")

    with open(path, "rb") as fp:
        header = load_header(fp)
    loadData = DATA_LOADERS[header.format]
    with open(path, "rb") as fp:
        fp.seek(header.dataOffset)
        data = loadData(fp, header)

    element_dict = {element.name: element for element in header.elements}

    # see about vertices ...
    assert "vertex" in element_dict
    vertex_element = element_dict["vertex"]
    vertex_props = {
        prop.name: pos for pos, prop in enumerate(vertex_element.properties)
    }
    assert "x" in vertex_props and "y" in vertex_props and "z" in vertex_props
    vertex_x = vertex_props["x"]
    vertex_y = vertex_props["y"]
    vertex_z = vertex_props["z"]

    # see about faces
    assert "face" in element_dict
    face_element = element_dict["face"]
    face_props = {prop.name: pos for pos, prop in enumerate(face_element.properties)}
    assert "vertex_indices" in face_props
    face_vertices = face_props["vertex_indices"]

    vertices = []
    for v in data["vertex"]:
        vertices.append((v[vertex_x], v[vertex_y], v[vertex_z]))

    triangles = []
    for face in data["face"]:
        indices = face[face_vertices]
        assert len(indices) >= 3
        for i in range(1, len(indices) - 1):
            triangles.append(((indices[0],), (indices[i],), (indices[i + 1],)))

    aabb_min = tuple([min([v[k] for v in vertices]) for k in range(3)])
    aabb_max = tuple([max([v[k] for v in vertices]) for k in range(3)])
    print(
        f"{len(vertices)} vertices, {len(triangles)} triangles, aabb [{aabb_min}, {aabb_max}]"
    )

    return liar.scenery.TriangleMesh(vertices, [], [], triangles)


def load_header(fp):
    fp = io.TextIOWrapper(fp, encoding="ascii", errors="replace")
    if fp.readline() != "ply\n":
        raise ValueError(f"not a PLY file")
    format = Format(fp.readline().strip())

    elements = []
    current_element = None

    while True:
        line = fp.readline()
        assert len(line) > 0

        splitted_line = line.split()
        command, fields = splitted_line[0], splitted_line[1:]
        num_fields = len(fields)

        if command == "end_header":
            assert num_fields == 0
            break

        if command == "element":
            assert num_fields == 2
            name = fields[0]
            number = int(fields[1])
            current_element = Element(name, number)
            elements.append(current_element)

        elif command == "property":
            assert num_fields > 0
            prop_type = fields[0]
            if prop_type == "list":
                assert num_fields == 4
                prop = Property(
                    prop_type, name=fields[3], num_type=fields[1], list_type=fields[2]
                )
            else:
                assert num_fields == 2
                prop = Property(prop_type, name=fields[1])
            current_element.properties.append(prop)

        elif command == "comment":
            pass

        else:
            raise ValueError(f"Unsupported command {command}")

    dataOffset = fp.tell()
    return Header(format, elements, dataOffset)


def load_data_ascii(fp, header: Header):
    fp = io.TextIOWrapper(fp, encoding="ascii")

    field_convertors = {
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

    ply_data = {}
    for element in header.elements:
        element_data = []
        for _index in range(element.number):
            line = fp.readline()
            assert len(line) > 0

            line_data = []
            fields = line.split()
            num_fields = len(fields)
            f = 0
            for prop in element.properties:
                if prop.type_ == "list":
                    assert f < num_fields
                    list_length = field_convertors[prop.num_type](fields[f])
                    f += 1
                    list_data = []
                    for k in range(list_length):
                        assert f < num_fields
                        list_data.append(field_convertors[prop.list_type](fields[f]))
                        f += 1
                    line_data.append(list_data)
                else:
                    assert prop.num_type is None
                    assert prop.list_type is None
                    assert f < num_fields
                    line_data.append(field_convertors[prop.type_](fields[f]))
                    f += 1
            assert f == num_fields

            element_data.append(line_data)

        ply_data[element.name] = element_data

    return ply_data


def load_data_binary(fp, header: Header):
    byte_order = "<" if header.format is Format.BINARY_LITTLE_ENDIAN_1_0 else ">"
    formats = {
        "int8": "b",
        "uint8": "B",
        "int16": "h",
        "uint16": "H",
        "int32": "i",
        "uint32": "I",
        "float32": "f",
        "float64": "d",
        "char": "b",
        "uchar": "B",
        "short": "h",
        "ushort": "H",
        "int": "i",
        "uint": "I",
        "float": "f",
        "double": "F",
    }

    def read(fp, type_, count=1):
        format = f"{byte_order}{count}{formats[type_]}"
        size = struct.calcsize(format)
        buffer = fp.read(size)
        return struct.unpack(format, buffer)

    ply_data = {}
    for element in header.elements:
        element_data = []
        for _index in range(element.number):
            line_data = []
            for prop in element.properties:
                if prop.type_ == "list":
                    (count,) = read(fp, prop.num_type)
                    values = read(fp, prop.list_type, count)
                    line_data.append(values)
                else:
                    (value,) = read(fp, prop.type_)
                    line_data.append(value)

            element_data.append(line_data)

        ply_data[element.name] = element_data

    return ply_data


DATA_LOADERS = {
    Format.ASCII_1_0: load_data_ascii,
    Format.BINARY_LITTLE_ENDIAN_1_0: load_data_binary,
    Format.BINARY_BIG_ENDIAN_1_0: load_data_binary,
}

# EOF

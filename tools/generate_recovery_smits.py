#! /usr/bin/env python

"""
Generate spectral data for Spectrum class using Smitt's algorithm

Using Brian Smits' algorithm described in "An RGB to Spectrum Conversion for Reflectances"
https://www.cs.utah.edu/~bes/papers/color/
"""

import os
import re

import scipy
import scipy.linalg
import scipy.optimize
from matplotlib import pyplot

import liar
import liar.tools.rgb_spaces


def main(
    num_bands,
    min_wavelength_nm,
    max_wavelength_nm,
    rgbSpaceName,
    niter=10000,
    fitall=False,
):
    observer = liar.Observer.standard()
    bands_nm = scipy.linspace(min_wavelength_nm, max_wavelength_nm, num_bands + 1)
    print(bands_nm)
    xyz_observer = scipy.transpose(
        scipy.array(
            [
                tristimulus_nm(observer, bands_nm[k], bands_nm[k + 1])
                for k in range(num_bands)
            ]
        )
    )

    rgb_space = get_rgb_space(rgbSpaceName)
    rgb_observer = xyz_to_rgb(xyz_observer, rgb_space)

    white, yellow, magenta, cyan, red, green, blue = generate_base_spectra(
        rgb_observer, niter, fitall
    )

    # write_code_fragments(bands, rgb_space, niter, fitall, xyz_observer, white, yellow, magenta, cyan, red, green, blue)

    if fitall:
        steps(bands_nm, white, "k")
        pyplot.figure()

    steps(bands_nm, red, "r")
    steps(bands_nm, green, "g")
    steps(bands_nm, blue, "b")
    steps(bands_nm, (red + green + blue), "k:")
    pyplot.figure()

    steps(bands_nm, yellow, "y")
    steps(bands_nm, magenta, "m")
    steps(bands_nm, cyan, "c")
    steps(bands_nm, (yellow + magenta + cyan) / 2, "k:")
    pyplot.show()


def tristimulus_nm(observer, w_min_nm, w_max_nm):
    X, Y, Z = 0, 0, 0
    for w_nm in range(int(w_min_nm), int(w_max_nm)):
        x, y, z = observer.sensitivity(w_nm * 1e-9)
        X += x * 1e-9
        Y += y * 1e-9
        Z += z * 1e-9
    return X, Y, Z


def get_rgb_space(name):
    rgb_space = liar.tools.rgb_spaces.__dict__.get(name)
    if not isinstance(rgb_space, liar.RgbSpace):
        raise KeyError("%s is not an RgbSpace in liar.tools.rgb_spaces" % name)
    return liar.RgbSpace(
        rgb_space.red, rgb_space.green, rgb_space.blue, (1.0 / 3, 1.0 / 3), 1.0
    )


def xyz_to_rgb(A, rgb_space):
    _, n = scipy.shape(A)
    return scipy.transpose([rgb_space.toRGBAlinear(A[:, k])[:3] for k in range(n)])


def generate_base_spectra(A, niter, fitall=False):
    null_A = null(A)
    pinv_A = scipy.linalg.pinv(A)

    _, n = scipy.shape(A)
    const_white = scipy.ones(n)

    white = fit(1, 1, 1, null_A, pinv_A, niter) if fitall else const_white
    yellow = fit(1, 1, 0, null_A, pinv_A, niter)
    magenta = fit(1, 0, 1, null_A, pinv_A, niter)
    cyan = fit(0, 1, 1, null_A, pinv_A, niter)
    red = fit(1, 0, 0, null_A, pinv_A, niter) if fitall else const_white - cyan
    green = fit(0, 1, 0, null_A, pinv_A, niter) if fitall else const_white - magenta
    blue = fit(0, 0, 1, null_A, pinv_A, niter) if fitall else const_white - yellow

    return white, yellow, magenta, cyan, red, green, blue


def null(A, eps=1e-15):
    # http://mail.scipy.org/pipermail/scipy-user/2005-June/004650.html
    # http://mail.scipy.org/pipermail/scipy-user/2008-December/019064.html
    n, m = scipy.shape(A)
    if n > m:
        return scipy.transpose(null(scipy.transpose(A), eps))
    u, s, vh = scipy.linalg.svd(A)
    assert m >= n
    s = scipy.append(s, [0] * (m - n))
    null_mask = s <= eps
    null_space = scipy.compress(null_mask, vh, axis=0)
    return scipy.transpose(null_space)


def fit(r, g, b, null_A, pinv_A, niter):
    print("fitting", r, g, b)
    rgb = scipy.array([r, g, b])
    x = scipy.dot(pinv_A, rgb)
    _, n = scipy.shape(null_A)
    s0 = [0.5] * n

    kwargs = {
        "method": "COBYLA",
        "args": (x, null_A),
        #'bounds': [(0,1)]*n,
        "constraints": [
            {
                "type": "ineq",
                "fun": lambda s, x, null_A: min(x + scipy.dot(null_A, s)),
                "args": (x, null_A),
            },
            {
                "type": "ineq",
                "fun": lambda s, x, null_A: 1.2 - max(x + scipy.dot(null_A, s)),
                "args": (x, null_A),
            },
        ],
        "options": {
            "disp": False,
        },
    }

    optimal = scipy.optimize.basinhopping(
        error, s0, minimizer_kwargs=kwargs, niter=niter
    )
    s = x + scipy.dot(null_A, optimal.x)

    # s = [max(0, min(v, 1)) for v in s]   # clamp between 0 and 1

    print(["%.4f" % f for f in s])
    return s


def error(s, x, null_A, *args):
    y = x + scipy.dot(null_A, s)
    mx = 100 * (max(y) - 1) if max(y) > 1 else 0  # penalty for values > 1
    # mx += (-1000 * min(y)) if min(y) < 0 else 0
    return 10 * scipy.linalg.norm(scipy.diff(y)) + mx


def write_code_fragments(
    bands, rgb_space, niter, fitall, A, white, yellow, magenta, cyan, red, green, blue
):
    self_dir = os.path.dirname(os.path.abspath(__file__))
    kernel_dir = os.path.join(os.path.dirname(self_dir), "src", "kernel")

    _, n = scipy.shape(A)

    meta_info = [
        "// Generated by %s with %d bands from %s to %s, using %s iterations, fitall=%s\n"
        % (os.path.basename(__file__), n, bands[0], bands[-1], niter, fitall)
    ]

    # spectrum.h
    block = ["enum { numBands = %d };\n" % n]
    replace_block(os.path.join(kernel_dir, "spectrum.h"), block, "numBands", meta_info)

    # spectrum.cpp
    chromaticities = (rgb_space.red, rgb_space.green, rgb_space.blue, rgb_space.white)

    block = ["TRgbSpacePtr rgbSpace(new RgbSpace(\n"]
    block += [
        "    TPoint2D(%s, %s),\n" % (floatstr(x), floatstr(y))
        for (x, y) in chromaticities
    ]
    block += ["    %s));\n" % floatstr(rgb_space.gamma)]

    block += array_block(
        "const XYZ A",
        ["XYZ(%s, %s, %s)" % tuple(map(floatstr, A[:, k])) for k in range(n)],
    )
    block += array_block("const TWavelength w", list(map(floatstr, bands)))
    block += array_block("const TScalar yellow", list(map(floatstr, yellow)))
    block += array_block("const TScalar magenta", list(map(floatstr, magenta)))
    block += array_block("const TScalar cyan", list(map(floatstr, cyan)))
    block += array_block("const TScalar red", list(map(floatstr, red)))
    block += array_block("const TScalar green", list(map(floatstr, green)))
    block += array_block("const TScalar blue", list(map(floatstr, blue)))
    replace_block(
        os.path.join(kernel_dir, "spectrum.cpp"), block, "initData", meta_info
    )


def replace_block(path, new_block_lines, name, meta_info):
    open_tag = "/* start autogen block %s */" % name
    close_tag = "/* end autogen block %s */" % name

    print("updating %s" % path)

    out = []
    with open(path, "r") as f:
        # search start of block
        for line in f:
            out.append(line)
            match = re.match(r"(\s*)%s\s*$" % re.escape(open_tag), line)
            if match:
                indentation = match.group(1)
                break
        else:
            raise RuntimeError("could not find start of autogen block %s" % name)

        # insert new block
        for line in meta_info + new_block_lines:
            out.append(indentation + line)

        # search end of block
        for line in f:
            match = re.match(r"(\s*)%s\s*$" % re.escape(close_tag), line)
            if match:
                out.append(line)
                break
        else:
            raise RuntimeError("could not find end of autogen block %s" % name)

        # add remainder of lines
        for line in f:
            out.append(line)

    with open(path, "w") as f:
        f.writelines(out)


def array_block(head, values):
    values = list(values)
    block = [
        "\n",
        head + "[%d] =\n" % len(values),
        "{\n",
    ]
    block += ["    %s,\n" % v for v in values]
    block += [
        "};\n",
    ]
    return block


def floatstr(x):
    return "%#gf" % x


def steps(Xs, Ys, c):
    pyplot.step(Xs, scipy.append(Ys, [0]), c, where="post")


# W_MIN, W_MAX, SIZE = 360e-9, 800e-9, 20
# W_MIN, W_MAX, SIZE = 380e-9, 720e-9, 10

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Generate the Spectrum<->XYZ conversion constants."
    )
    parser.add_argument(
        "-n",
        "--num-bands",
        type=int,
        default=10,
        metavar="<n>",
        help="number of bands in spectrum [default=%(default)s]",
    )
    parser.add_argument(
        "-w",
        "--w-min",
        type=float,
        default=380,
        metavar="<w>",
        help="lower wavelength [nm] bound of spectrum [default=%(default)s]",
    )
    parser.add_argument(
        "-W",
        "--w-max",
        type=float,
        default=720,
        metavar="<w>",
        help="upper wavelength [nm] bound of spectrum [default=%(default)s]",
    )
    parser.add_argument(
        "--niter",
        type=int,
        default=10000,
        metavar="<n>",
        help="number of iterations in optimization algorithm, more is better [default=%(default)s]",
    )
    parser.add_argument(
        "--rgb-space",
        type=str,
        default="sRGB",
        metavar="<name>",
        help="RGB space to be used, name of one of the spaces in liar.rgb_spaces [default=%(default)s]",
    )
    parser.add_argument(
        "--fit-all",
        default=False,
        action="store_true",
        help="Also fit white, red, green and blue instead of just yellow, magenta and cyan.",
    )
    args = parser.parse_args()
    main(
        args.num_bands, args.w_min, args.w_max, args.rgb_space, args.niter, args.fit_all
    )

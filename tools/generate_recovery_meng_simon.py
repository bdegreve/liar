#! /usr/bin/env python

import scipy
import scipy.optimize
import numpy as np
import numpy.linalg as linalg
import numpy.ma
import colour.plotting
import colour.difference.delta_e
import colour.models.cie_lab
import pylab
import json
import liar
import liar.tools.rgb_spaces


def main(out, min_dist, step_size, nudge_factor, niter, ftol, plot):
    observer = liar.Observer.standard()

    border, inside = generate_sample_points(observer, min_dist, step_size, nudge_factor)
    print '#border:', len(border)
    print '#inside:', len(inside)
    if plot:
        plot_sample_points(border, inside)

    xy_points = border + inside
    wavelengths = observer.wavelengths
    spectra = generate_spectra(observer, xy_points, niter, ftol)
    if plot:
        plot_spectra(spectra, wavelengths)
    if out:
        save_spectra(out, wavelengths, spectra)

    test(wavelengths, spectra, observer)

    if plot:
        pylab.show()


def generate_sample_points(observer, min_dist, step_size, nudge_factor):
    ws = np.array(observer.wavelengths)
    xys = map(xy_from_XYZ, observer.sensitivities)

    xy_white = np.array([1./3, 1./3])
    xys = [nudge_to_white(xy, xy_white, nudge_factor) for xy in xys]

    w_green = 525
    _, k_green = min((abs(w - w_green), k) for k, w in enumerate(ws))
    xy_green = xys[k_green]

    border = generate_border_samples(xys, k_green, min_dist)
    inside = generate_inside_samples(border, xy_white, xy_green, min_dist, step_size)

    return border, inside


def generate_border_samples(xys, k_green, min_dist):
    xy_blue, xy_red, xy_green = xys[0], xys[-1], xys[k_green]

    border_blue = [xy_blue]
    for xy in xys[1:k_green]:
        if dist_xy(xy, border_blue[-1]) < min_dist:
            continue
        if dist_xy(xy, xy_green) < min_dist:
            continue
        border_blue.append(xy)

    border_red = [xy_red]
    for xy in reversed(xys[k_green+1:-1]):
        if dist_xy(xy, border_red[-1]) < min_dist:
            continue
        if dist_xy(xy, xy_green) < min_dist:
            continue
        border_red.append(xy)
    border_red.reverse()

    return border_blue + [xy_green] + border_red


def generate_inside_samples(border, xy_white, xy_green, min_dist, step_size):
    xy_blue, xy_red = border[0], border[-1]
    du, dv = xy_red - xy_blue, xy_green - xy_blue
    nu, nv = int(round(linalg.norm(du) / step_size)), int(round(linalg.norm(dv) / step_size))
    du, dv = du / nu, dv / nv

    inside = [xy_white]
    for u in range(-nu, 2 * nu):
        for v in range(0, nv + 1):
            xy = xy_blue + u * du + v * dv
            if dist_xy(xy, xy_white) < min_dist:
                continue
            if any(dist_xy(xy, xy_b) < min_dist for xy_b in border):
                continue
            if not is_inside_cw(xy, border) and not (v == 0 and u >= 0 and u <= nu):
                continue
            inside.append(xy)

    return inside


def generate_spectra(observer, xy_points, niter, ftol):
    return { tuple(xy): fit_spectrum(observer, xy, niter, ftol) for xy in xy_points }


def fit_spectrum(observer, xy, niter, ftol):
    print "---", xy
    n = len(observer.wavelengths)
    x0 = np.array([0] * n)
    bounds = [(0, 1000)] * n
    xyz = xyz_from_xy(xy)

    constraint = {
        'type': 'eq',
        'fun': lambda s: np.array(observer.tristimulus(s)) - xyz
    }
    options = {
        'maxiter': niter,
        'ftol': ftol,
        'disp': True
    }
    optimal = scipy.optimize.minimize(error, x0, method='SLSQP', jac=jac, bounds=bounds, constraints=constraint, options=options)
    spectrum = np.array([max(s, 0) for s in optimal.x])

    #print spectrum
    print 'max:', max(spectrum)
    print 'delta E:', delta_E_CIE1976(xyz, observer.tristimulus(spectrum))

    return spectrum


def error(spectrum):
    return sum((a - b) ** 2 for a, b in zip(spectrum[:-1], spectrum[1:]))


def jac(spectrum):
    x1, x2 = spectrum[:2]
    dx1 = 2 * (x1 - x2)
    xm, xn = spectrum[-2:]
    dxn = 2 * (xn - xm)
    j = [dx1] + [4 * spectrum[k] - 2 * (spectrum[k - 1] + spectrum[k + 1]) for k in range(1, len(spectrum) - 1)] + [dxn]
    assert len(j) == len(spectrum)
    return np.array(j)


def test(wavelengths, spectra, observer):
    recovery = liar.spectra.RecoveryMengSimon(wavelengths, spectra)
    edges = recovery.meshEdges()

    rgb_space = liar.tools.rgb_spaces.AdobeRGB #liar.sRGB
    xy_red, xy_green, xy_blue = np.array(rgb_space.red), np.array(rgb_space.green), np.array(rgb_space.blue)
    polygon_cw = [xy_blue, xy_green, xy_red]
    x_min = min(xy_red[0], xy_green[0], xy_blue[0])
    x_max = max(xy_red[0], xy_green[0], xy_blue[0])
    y_min = min(xy_red[1], xy_green[1], xy_blue[1])
    y_max = max(xy_red[1], xy_green[1], xy_blue[1])
    n = 200
    dx, dy = (x_max - x_min) / (n - 1), (y_max - y_min) / (n - 1)

    ys, xs = np.mgrid[slice(y_min, y_max + dy, dy), slice(x_min, x_max + dx, dx)]
    ny, nx = xs.shape
    es = np.zeros((ny, nx))

    max_delta_e, max_xy = 0, None
    for i in range(ny - 1):
        for j in range(nx - 1):
            x, y = xs[i, j], ys[i, j]
            if not is_inside_cw((x, y), polygon_cw):
                es[i, j] = -1
                continue
            z = 1 - x - y
            assert(z > 0)
            xyz = np.array([x, y, z])
            spectrum = recovery.recover(xyz)
            xyz_prime = observer.tristimulus(spectrum)
            es[i, j] = delta_E_CIE1976(xyz, xyz_prime)
    print "max delta E:", np.max(es)

    pylab.figure()
    colour.plotting.CIE_1931_chromaticity_diagram_plot(standalone=False)
    es = es[:-1, :-1]
    es = numpy.ma.masked_array(es, es < 0)
    pylab.pcolormesh(xs, ys, es)
    pylab.colorbar()

    xs, ys = [], []
    for (xa, ya), (xb, yb) in edges:
        xs += [xa, xb, None]
        ys += [ya, yb, None]
    pylab.xlim([min(x for x in xs if x is not None), max(x for x in xs if x is not None)])
    pylab.ylim([min(y for y in ys if y is not None), max(y for y in ys if y is not None)])
    pylab.plot(xs, ys, '-k')
    xs, ys = zip(*spectra.keys())
    pylab.plot(xs, ys, 'ow')


def delta_E_CIE1976(xyz_a, xyz_b):
    xy_white = (1./3, 1./3)
    lab_a = colour.models.cie_lab.XYZ_to_Lab(xyz_a, xy_white)
    lab_b = colour.models.cie_lab.XYZ_to_Lab(xyz_b, xy_white)
    return colour.difference.delta_e.delta_E_CIE1976(lab_a, lab_b)


def plot_sample_points(border, inside):
    pylab.figure()
    colour.plotting.CIE_1931_chromaticity_diagram_plot(standalone=False)
    xs, ys = zip(*border)
    pylab.plot(xs, ys, '-ow')
    xs, ys = zip(*inside)
    pylab.plot(xs, ys, 'ow')


def plot_spectra(spectra, wavelengths):
    pylab.figure()
    for spectrum in spectra.itervalues():
        pylab.plot(wavelengths, spectrum)


def save_spectra(path, wavelengths, spectra):
    with open(path, 'w') as f:
        json.dump({
            'wavelengths': tuple(wavelengths),
            'spectra': [{"xy": xy, "spectrum": tuple(s)} for xy, s in spectra.iteritems()]
        },
        f, indent=2)


def load_spectra(path):
    with open(path, 'r') as f:
        data = json.load(f)
    wavelengths = data['wavelengths']
    spectra = {tuple(s['xy']): s['spectrum'] for s in data['spectra']}
    return wavelengths, spectra


def xy_from_XYZ(xyz):
    x, y, z = xyz
    s = x + y + z
    assert s > 0
    return np.array([x / s, y / s])


def xyz_from_xy(xy):
    x, y = xy
    z = 1 - x - y
    assert z > 0
    return np.array([x, y, z])


def dist_xy(xy_a, xy_b):
    return linalg.norm(xy_a - xy_b)


def normalize_xy(xy):
    return xy / linalg.norm(xy)


def is_inside_cw(xy, polygon_cw):
    for k, xy_b in enumerate(polygon_cw):
        xy_a = polygon_cw[k - 1]
        if perp_dot(xy_b - xy_a, xy - xy_a) > 0:
            return False
    return True


def perp_dot(xy_a, xy_b):
    return xy_a[0] * xy_b[1] - xy_a[1] * xy_b[0]


def nudge_to_white(xy, xy_white, factor):
    return xy * (1 - factor) + xy_white * factor


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Generate the Spectrum<->XYZ conversion constants for Meng Simon model.')
    parser.add_argument('--min-dist', type=float, default=0.02, metavar='<d>', help='minumum xy distance between two points [default=%(default)s]')
    parser.add_argument('--step-size', type=float, default=0.075, metavar='<d>', help='step size of triangular grid [default=%(default)s]')
    parser.add_argument('--nudge-factor', type=float, default=0.03, metavar='<f>', help='factor to ease border towards white [default=%(default)s]')
    parser.add_argument("--niter", type=int, default=10000, metavar='<n>', help='max number of iterations in optimization algorithm, more is better [default=%(default)s]')
    parser.add_argument("--ftol", type=float, default=1e-10, metavar='<f>', help='tolerance, smaller is better [default=%(default)s]')
    parser.add_argument('-o', '--out', type=str, default='recovery_meng_simon.json', metavar='<filename>', help='output file [default=%(default)s]')
    parser.add_argument('--plot', default=False, action='store_true', help='plot results')
    args = parser.parse_args()
    main(args.out, args.min_dist, args.step_size, args.nudge_factor, args.niter, args.ftol, args.plot)

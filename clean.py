# deletes all builded files of all projects

import os
import os.path

extensions_to_be_deleted = ['obj', 'lib', 'exe', 'dll', 'pyd', 'pdb', 'pch', 'exp', 'sbr', 'idb', 'ilk', 'ncb', 'log', 'xml', 'pdf', 'aux', 'bbl', 'dvi', 'ps', 'blg', 'toc', 'tps']

def deletor(dummy, dirname, files):
    for f in files:
        ext = os.path.splitext(f)[1][1:].lower()
        if ext in extensions_to_be_deleted:
            path = os.path.join(dirname, f)
            print path,
            try:
                os.remove(path)
                print "ok"
            except:
                print "FAILED"

os.path.walk('.', deletor, None)

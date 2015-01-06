'''
   export_to_pickle

   export observations in a stream file to pickle files
   cctbx.xfel can read.

   WARNING: This program is NOT tested throughfully and NOT working well.

'''

from cctbx.array_family import flex
from cctbx import miller, crystal, sgtbx, uctbx, crystal_orientation
from scitbx import matrix
import cPickle as pickle
import sys

# reference http://cctbx.sourceforge.net/siena2005/sort_merge.py

# This will be overwritten by values in .phil but still you need it!
# This field in pickle is named 'pointgroup' but actually seems to be space group...
Pointgroup = "4mm"
Spacegroup = "I422" # TODO: use this
Distance = 51.5

i_cryst = 0

def nextline():
    return sys.stdin.readline()

while (True):
    inp = nextline().strip()
    if (inp == ""):
        break
    print inp
    i_cryst += 1

    inp = nextline()
    wavelength = float(inp)
    tmp = [float(x) * 1E-10 for x in nextline().split()]

    #    flip x and y-axis
    #    tmp[0], tmp[3], tmp[6], tmp[1], tmp[4], tmp[7] = tmp[1], tmp[4], tmp[7], tmp[0], tmp[3], tmp[6], 

    # flip the z-axis
    # FIXME: Check if this is correct.
    tmp[2] *= -1
    tmp[5] *= -1
    tmp[8] *= -1

    reciprocal_basis = matrix.sqr(tmp).transpose()
    current_orientation = crystal_orientation.crystal_orientation(reciprocal_basis, 
                                                                  crystal_orientation.basis_type.reciprocal)
    n_reflections = int(nextline())
    
    data = flex.double()
    sigmas = flex.double()
    miller_indices = flex.miller_index()
    mapped_predictions = flex.vec2_double()
    
    for i in xrange(0, n_reflections):
        fields = nextline().split()
        miller_indices.append([int(value) for value in fields[:3]])
        data.append(float(fields[3]))
        sigmas.append(float(fields[4]))
        mapped_predictions.append([float(fields[5]), float(fields[6])])
    
    point_group = Pointgroup
    crystal_symmetry = crystal.symmetry(unit_cell=current_orientation.unit_cell(),
                                        space_group = sgtbx.space_group())
        
    miller_set = miller.set(crystal_symmetry=crystal_symmetry,
                            indices=miller_indices,
                            anomalous_flag=False)
    observations = miller.array(miller_set, data=data,
                                sigmas=sigmas)
        
    save_to_pickle = {'xbeam': 0, 'ybeam': 0, 'wavelength': wavelength,
                      'pointgroup': point_group,
                      'mapped_predictions': [mapped_predictions],
                      'observations': [observations], 'distance': Distance,
                      'current_orientation': [current_orientation]}
        
    pickle.dump(save_to_pickle, open("%d.pickle" % i_cryst, "wb"))



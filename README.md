CrystFEL
========

This is a personal **unofficial** repository of CrystFEL for processing serial crystallography dataset.
The official CrystFEL website is http://www.desy.de/~twhite/crystfel/ and the repository is 
https://stash.desy.de/projects/CRYS/repos/crystfel/browse.

As of 2016, most features have been merged to the official repository, except for hdfsee improvements.
Thus, I do not update this repository very often.

Warning!
--------

I am testing various ideas on this repository. 
Some are not stable yet and **NOT** recommended for production work. 
Use at your own risk!

Added Features
==============

multievent branch
-----------------

This is personal "master" branch where developments take place.
"master" branch tracks the official repository.
hdfsee and multiple-lattice branches have been merged here.

* More updates in the stream viewer (hdfsee)
    * Resizable columns
    * Sortable rows
    * Support multi-event HDF files

experimental branch
-------------------

Highly premature, experimental codes. Do **NOT** use this branch!

* get_rms  
  calculate RMS between observed spot positions and predicted positions.
* export_to_pickle  
  convert a stream file to pickle files cctbx.xfel can read.  
  very buggy, not working well.
* dump_unmerged  
  dump unmerged (but indices converted to be in the ASU) intensities from 
  a stream file for statistical inspection.

hdfsee improvements 
-------------------

Should be stable enough for general use. Merged to multievent branch.

* Load stream file directly into hdfsee
* Show spots and (multiple) lattices simultaneously
* Show resolution and Miller index to the status bar by clicking
* Scrollbar in hdfsee -> merged to the official distribution
* Export to ADSC format. -> merged to the official distribution  
  This is useful for manual inspection of indexing in iMOSFLM.
  Since the original pixel doesn't have one-to-one correspondence in
  the output image, this is not perfectly accurate.

multiple-lattice indexing
-------------------------

This feature has been merged into the multievent branch.
Seems stable but please carefully examine if it improves the result.

* Multiple lattice indexing by calling external indexer(s) many times.
* Sending prior-cell information to MOSFLM.  
  Prior-cell algorithm will be available in the next version of MOSFLM.
* Hitrate calculator based on Poisson distribution in doc/hitrate.html

scaling
-------

All features have been merged to the official distirubtion. Thus this branch was deleted.

* Calculate CC between reference and each frame
* Frame rejection based on CC
* Output CC and scale factor from process_hkl to be plotted in R



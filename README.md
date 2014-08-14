CrystFEL
========

This is a personal **unofficial** repository of CrystFEL for processing serial crystallography dataset.
The official CrystFEL website is http://www.desy.de/~twhite/crystfel/ and the repository is  http://git.bitwiz.org.uk/?p=crystfel.git.

Warning!
--------

I am testing various ideas on this repository. 
Some are not stable yet and **NOT** recommended for publication work. 
Use at your own risk!

Added Features
==============

hdfsee branch
-------------

Should be stable enough for general use.

*   Load stream file directly into hdfsee
*   Show spots and (multiple) lattices simultaneously
*   Scrollbar in hdfsee -> merged to the official distribution
*   Export to ADSC format. -> merged to the official distribution  
    This is useful for manual inspection of indexing in iMOSFLM.
    Since the original pixel doesn't have one-to-one correspondence in
    the output image, this is not perfectly accurate.

multiple-lattice branch
-----------------------

Seems stable but please carefully examine if it improves the result.

* Multiple lattice indexing by calling external indexer(s) many times.

scaling branch
--------------

Should be stable enough.

* Calculate CC between reference and each frame
* Frame rejection based on CC
* Output CC and scale factor from process_hkl to be plotted in R


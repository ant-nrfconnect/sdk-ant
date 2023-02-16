# ANT documentation build configuration file

import os
from pathlib import Path
import sys
import subprocess

# -- Paths --------------------------------------------------------------

NRF_BASE = Path(__file__).absolute().parents[2].joinpath("nrf")

sys.path.insert(0, str(NRF_BASE / "doc" / "_utils"))
import utils

#ZEPHYR_BASE = utils.get_projdir("zephyr")

# Doxygen
subprocess.call('doxygen Doxyfile.in', shell=True)

# -- Project information -----------------------------------------------------

project = 'ANT for nRF Connect SDK'
copyright = '2022, Garmin Canada Inc'
author = 'Garmin Canada Inc.'
version = '0.5.0'

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'breathe',
    'sphinx_rtd_theme',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# -- Options for HTML output -------------------------------------------------

html_theme = "sphinx_rtd_theme"

html_static_path = ['_static']
html_css_files = ['custom.css']
html_logo = "this-is-ant.png"
html_favicon = 'favicon.ico'
html_theme_options = {
    'display_version': True,
}

# -- Breathe configuration -------------------------------------------------

breathe_projects = {
   "ANT for nRF Connect SDK": "_build/xml/"
}
breathe_default_project = "ANT for nRF Connect SDK"
breathe_default_members = ('members', 'undoc-members')
breathe_show_define_initializer = True
breathe_show_enumvalue_initializer = True
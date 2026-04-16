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
copyright = '2026, Garmin Canada Inc'
author = 'Garmin Canada Inc.'
version = '2.1.0'
past_versions = ['2.0.0', '1.3.0', '1.2.0', '1.1.0', '1.0.0', '0.5.0']

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
breathe_domain_by_extension = {"h": "c", "c": "c"}

# -- Previous versions -----------------------------------------------------
try:
   html_context
except NameError:
   html_context = dict()
html_context['display_lower_left'] = True

templates_path = ['_templates']

# tell the theme which version we're currently on ('current_version' affects
# the lower-left rtd menu and 'version' affects the logo-area version)
html_context['current_version'] = version
html_context['version'] = version

# POPULATE LINKS TO OTHER VERSIONS
html_context['versions'] = list()
html_context['versions'].append( (version, 'https://ant-nrfconnect.github.io/') )

for past_version in past_versions:
   html_context['versions'].append( (past_version, 'https://ant-nrfconnect.github.io/versions/v' + past_version + '/') )

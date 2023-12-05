#!/usr/bin/env python
####
# baremetal-size Python Package:
#
# This package allows you to view the static data sizes of a binary file:
#
# User Install / Upgrade:
# ```
# pip install --upgrade .
# ```
#
###
from setuptools import find_packages, setup

# Setup a python package using setup-tools. This is a newer (and more recommended) technology
# then distutils.
setup(
    ####
    # Package Description:
    #
    # Basic package information. Describes the package and the data contained inside. This
    # information should match the F prime description information.
    ####
    name="baremetal-size",
    use_scm_version={"root": ".", "relative_to": __file__},
    license="Apache 2.0 License",
    description="F Prime Sizing Utility",
    long_description="""
This package allows you to view the static data sizes of a binary file.
    """,
    url="https://github.com/ethancheez/fprime-baremetal",
    keywords=["fprime", "embedded", "nasa"],
    project_urls={"Issue Tracker": "https://github.com/ethancheez/fprime-baremetal/issues"},
    # Author of Python package, not F prime.
    author="Ethan Chee",
    author_email="ethancheez@gmail.com",
    ####
    # Included Packages:
    #
    # Will search for and included all python packages under the "src" directory.  The root package
    # is set to 'src' to avoid package names of the form src.baremetal.
    ####
    packages=find_packages("src"),
    package_dir={"": "src"},
    ####
    # Classifiers:
    #
    # Standard Python classifiers used to describe this package.
    ####
    classifiers=[
        # complete classifier list: http://pypi.python.org/pypi?%3Aaction=list_classifiers
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Operating System :: Unix",
        "Operating System :: POSIX",
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: Implementation :: CPython",
        "Programming Language :: Python :: Implementation :: PyPy",
    ],
    # Requires Python 3.7+
    python_requires=">=3.7",
    install_requires=[],
    extras_require={},
    # Setup and test requirements, not needed by normal install
    setup_requires=[],
    tests_require=[],
    # Create a set of executable entry-points for running directly from the package
    entry_points={
        "console_scripts": [
            "baremetal-size = baremetal.size.__main__:main",
        ],
        "gui_scripts": [],
    },
)

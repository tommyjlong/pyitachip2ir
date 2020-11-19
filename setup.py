"""A setuptools based setup module.
See:
https://packaging.python.org/en/latest/distributing.html
https://github.com/alanfischer/itachip2ir
"""

from setuptools import setup, Extension
from codecs import open
from os import path

here = path.abspath(path.dirname(__file__))

with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
    long_description = f.read()

module = Extension('itachip2ir',
                    sources = [
                        'source/ITachIP2IR.cpp',
                        'source/IRCommandParser.cpp'
                    ])

setup(
    name='pyitachip2ir',
    version='0.0.7',
    description='A library for sending IR commands to an ITach IP2IR gateway',
    long_description=long_description,
    url='https://github.com/alanfischer/itachip2ir',
    author='Alan Fischer',
    author_email='alan@lightningtoads.com',
    license='MIT',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Topic :: Home Automation',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
    ],
    keywords='itach ip2ir homeautomation',
    py_modules = ["pyitachip2ir"],
    ext_modules = [module]
)

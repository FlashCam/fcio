from setuptools import setup
from setuptools.extension import Extension

extensions = [
    Extension(
        'fcio.fcio_c',
        sources=[
            'src/fcio/src/fcio.c',
            'src/fcio/src/time_utils.c',
            'src/fcio/src/tmio.c',
            'src/fcio/src/bufio.c',
        ],
        include_dirs=['src/fcio/include'],
        language='c',
        # extra_compile_args=extra_compile_args(),
    )
]

setup(ext_modules=extensions)

from distutils.core import setup

setup(
    name            = 'SRL',
    version         = '0.1.0',
    author          = 'Anthony Wertz',
    author_email    = 'awertz@cmu.edu',
    packages        = ['srl', 'srl.test'],
    scripts         = [],
    url             = '',
    license         = 'LICENSE.txt',
    description     = 'SRL interface library.',
    long_description= open('README.txt').read(),
    )

from fabric.api import *
from fabric.colors import *

# output colors in different color.
env.colorize_errors = True

# run all commands in //
env.parallel = True

# Only prints a warning when a command fails.
env.warn_only = True

env.user = 'student'
env.roledefs = {
    'm200s': [
        '192.168.5.100',
        '192.168.6.100',
        # '192.168.7.100',
        ],
    'mokais': [
        '192.168.1.191',
        '192.168.1.192',
        ],
}

aqua_dir = '/home/student/moos-ivp-aquaticus/'


def cd_aquaticus():
    cd(aqua_dir)


def update_trunk():
    run('svn up')


def cd_missions():
    cd(os.path.join(aqua_dir, 'missions'))


def cd_mission(name):
    cd_missions
    cd(name)


@roles('mokais', 'm200s')
def prepare_deploy():
    print(blue('Executing on ') + red('%(host)s' % env) + '.')
    run('pwd')


@roles('mokais')
def build_aquaticus():
    print(blue('Executing on ') + red('%(host)s' % env) + '.')
    run('ifconfig eth0')


def run_all():
    execute(prepare_deploy)

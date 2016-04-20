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
moosivp_dir = '/home/student/moos-ivp/'


def svn_up():
    run('svn up')


def update_trunk(path):
    with cd(path):
        svn_up()


def build_trunk(path):
    with cd(path):
        run('./build.sh')


def cd_missions():
    cd(os.path.join(aquaticus, 'missions'))


def cd_mission(name):
    cd_missions()
    cd(name)


@roles('mokais')
def build_aquaticus():
    print(green('building aquaticus on ') + red('%(host)s' % env) + '.')
    update_trunk(aqua_dir)
    build_trunk(aqua_dir)
    print(green('done building aquaticus on ') + red('%(host)s' % env) + '.')


@roles('mokais')
def build_moosivp():
    print(green('building moos-ivp') + red('%(host)s' % env) + '.')
    update_trunk(moosivp_dir)
    build_trunk(moosivp_dir)
    print(green('done building moos-ivp') + red('%(host)s' % env) + '.')


@task
def run_all():
    execute(build_moosivp)
    execute(build_aquaticus)

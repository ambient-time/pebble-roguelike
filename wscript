# Copyright 2026 Luke Steuber. MIT License.
top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')
    original = ctx.env
    binaries = []
    for platform in ctx.env.TARGET_PLATFORMS:
        ctx.env = ctx.all_envs[platform]
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_build(source=ctx.path.ant_glob('src/c/**/*.c'), target=app, bin_type='app')
        binaries.append({'platform': platform, 'app_elf': app})
    ctx.env = original
    ctx.set_group('bundle')
    ctx.pbl_bundle(binaries=binaries)

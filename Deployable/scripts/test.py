from valkyrie import *

script_info = {
    'author': 'leryss',
    'description': 'none',
    'name': 'Pretty Test'
}

test = True

def valkyrie_menu(ctx):
    global test
    test = ctx.ui.checkbox("interesting", test)
    
def valkyrie_on_load(ctx):
    ctx.log(ctx.log.__doc__)

def valkyrie_exec(ctx):
    return
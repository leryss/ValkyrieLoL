from valkyrie import *

script_info = {
    'author': 'leryss',
    'description': 'none',
    'name': 'Pretty Test'
}

def valkyrie_menu(ctx):
    ctx.log("hello menu")
    
def valkyrie_on_load(ctx):
    ctx.log("hello on load")

def valkyrie_exec(ctx):
    return
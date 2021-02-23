from valkyrie import *
import json

class KeyInput:

	def __init__(self, key, is_held):
		self.key      = key
		self.is_held  = is_held
		self.toggled  = False
	
	def ui(self, label, ui):
		ui.pushid(id(self))
		
		ui.separator()
		ui.text(label)
		self.key = ui.keyselect('Key', self.key)
		self.is_held = ui.checkbox("Hold mode", self.is_held)
		
		ui.popid()
		
	def check(self, ctx):
		if self.is_held:
			return ctx.is_held(self.key)
		else:
			if ctx.was_pressed(self.key):
				self.toggled = not self.toggled
			
			return self.toggled
		
	def __str__(self):
		return json.dumps([self.key, self.is_held])

	@classmethod
	def from_str(self, s):
		j = json.loads(s)
		
		return KeyInput(*j)
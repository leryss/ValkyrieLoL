from valkyrie import *			 
from helpers.flags import EvadeFlags
import time, json

RADIANS_90_DEG = 1.57079633

move_cooldown  = 0.1
last_moved = 0

class EvadePriority:
	Lowest  = 0
	Low     = 1
	Normal  = 2
	High    = 3
	Highest = 4

class EvadeSettings:
	
	PriorityNames = {
		EvadePriority.Lowest : 'Lowest', 
		EvadePriority.Low    : 'Low',  
		EvadePriority.Normal : 'Normal', 
		EvadePriority.High   : 'High',   
		EvadePriority.Highest: 'Highest'
	}
	
	def __init__(self, name = '', cast_names = [], missile_names = [], evade = True, evade_cast = True, priority = 0):
		self.name = name
		self.cast_names = cast_names
		self.missile_names = missile_names
		
		self.evade = evade
		self.evade_cast = evade_cast
		self.priority = priority
		
	def ui(self, ui):
		ui.text(self.name, Col.Purple)
		
		self.evade = ui.checkbox('Evade', self.evade)
		if len(self.cast_names) > 0:
			self.evade_cast = ui.checkbox('Instant reaction', self.evade_cast)
		
		self.priority = ui.sliderenum('Priority', self.PriorityNames[self.priority], self.priority, EvadePriority.Highest)
		#self.priority = ui.sliderint('Priority', self.priority, 0, 5)
			
	def __str__(self):
		return json.dumps({
			'name': self.name,
			'casts': self.cast_names,
			'missiles': self.missile_names,
			'evade': self.evade,
			'evade_c': self.evade_cast,
			'prio': self.priority
		})
		
	@classmethod
	def from_str(self, s):
		d = json.loads(s)
		return EvadeSettings(
			name          = d.get('name', 'No name'),
			cast_names    = d.get('casts', []),
			missile_names = d.get('missiles', []),
			evade         = d.get('evade', True),
			evade_cast    = d.get('evade_c', True),
			priority      = d.get('prio', 0)
		)

enabled            = True
always_evade       = False
extra_evade_length = 50.0

NotSupportedYet = ['jayce', 'kennen', 'kogmaw', 'malzahar', 'maokai', 'ornn', 'reksai', 'rumble', 'skarner' , 'tahmkench', 'urgot', 'varus', 'viego']
Settings = {
	'aatrox' : [
		EvadeSettings(name = 'Aaatrox W',             cast_names = ['aatroxw'],                   missile_names = ['aatroxw'])
	],                                                                                            
	'ahri' : [                                                                                    
		EvadeSettings(name = 'Ahri Q',                cast_names = ['ahriorbofdeception'],        missile_names = ['ahriorbmissile']),
		EvadeSettings(name = 'Ahri E',                cast_names = ['ahriseduce'],                missile_names = ['ahriseducemissile'])
	],                                                                                            
	'akali': [                                                                                    
		EvadeSettings(name = 'Akali Q',               cast_names = ['akaliq'],                    missile_names = []),
		EvadeSettings(name = 'Akali E',               cast_names = ['akalie'],                    missile_names = ['akaliemis'])
	],                                                                                            
	'amumu': [                                                                                    
		EvadeSettings(name = 'Amumu Q',               cast_names = ['bandagetoss'],               missile_names = ['sadmummybandagetoss']),
	],                                                                                            
	'anivia': [                                                                                   
		EvadeSettings(name = 'Anivia Q',              cast_names = ['flashfrostspell'],           missile_names = ['flashfrostspell'])
	],                                                                                            
	'annie': [                                                                                    
		EvadeSettings(name = 'Annie W',               cast_names = ['anniew'],                    missile_names = []),
		EvadeSettings(name = 'Annie R',               cast_names = ['annier'],                    missile_names = [])
	],                                                                                            
	'aphelios': [                                                                                 
		EvadeSettings(name = 'Aphelios Calibrum Q',   cast_names = ['aphelioscalibrumq'],         missile_names = ['aphelioscalibrumq']),
		EvadeSettings(name = 'Aphelios R',            cast_names = ['apheliosr'],                 missile_names = ['apheliosrmis'])
	],
	'ashe': [
		EvadeSettings(name = 'Ashe W',                cast_names = ['volley', 'volleyrank2', 'volleyrank3', 'volleyrank4', 'volleyrank5'], missile_names = ['volleyattack', 'volleyattackwithsound']),
		EvadeSettings(name = 'Ashe R',                cast_names = ['enchantedcrystalarrow'],     missile_names = ['enchantedcrystalarrow'])
	],                                                                                            
	'aurelionsol': [                                                                              
		EvadeSettings(name = 'Aurelion R',            cast_names = ['aurelionsolr'],              missile_names = ['aurelionsolrbeammissile'])
	],                                                                                            
	'bard': [                                                                                     
		EvadeSettings(name = 'Bard Q',                cast_names = ['bardq'],                     missile_names = ['bardqmissile']),
		EvadeSettings(name = 'Bard R',                cast_names = ['bardr'],                     missile_names = ['bardrmissilefixedtraveltime'])
	],                                                                                            
	'blitzcrank': [                                                                               
		EvadeSettings(name = 'Blitzcrank',            cast_names = ['rocketgrab'],                missile_names = ['rocketgrabmissile'])
	],                                                                                            
	'brand': [                                                                                    
		EvadeSettings(name = 'Brand Q',               cast_names = ['brandq'],                    missile_names = ['brandqmissile']),
		EvadeSettings(name = 'Brand W',               cast_names = ['brandw'],                    missile_names = [])
	],                                                                                            
	'braum': [                                                                                    
		EvadeSettings(name = 'Braum Q',               cast_names = ['braumq'],                    missile_names = ['braumqmissile']),
		EvadeSettings(name = 'Braum R',               cast_names = ['braumrwrapper'],             missile_names = ['braumrmissile'])
	],
	'caitlyn': [
		EvadeSettings(name = 'CaitlynQ',              cast_names = ['caitlynpiltoverpeacemaker'], missile_names = ['caitlynpiltoverpeacemaker']),
		EvadeSettings(name = 'CaitlynE',              cast_names = ['caitlynentrapment'],         missile_names = ['caitlynentrapmentmissile'])
	],
	'cassiopeia': [
		EvadeSettings(name = 'Cassiopeia Q',          cast_names = ['cassiopeiaq'],               missile_names = []),
		EvadeSettings(name = 'Cassiopeia R',          cast_names = ['cassiopeiar'],               missile_names = [])
	],
	'chogath': [
		EvadeSettings(name = 'Chogath Q',             cast_names = ['rupture'],                   missile_names = []),
		EvadeSettings(name = 'Chogath W',             cast_names = ['feralscream'],               missile_names = [])
	],
	'corki': [
		EvadeSettings(name = 'Corki Q',               cast_names = ['phosphorusbomb'],            missile_names = ['phosphorusbombmissile']),
		EvadeSettings(name = 'Corki R',               cast_names = ['missilebarragemissile', 'missilebarragemissile2'], missile_names = ['missilebarragemissile', 'missilebarragemissile2'])
	],
	'darius': [
		EvadeSettings(name = 'Darius E',              cast_names = ['dariusaxegrabcone'],         missile_names = [])
	],
	'diana': [
		EvadeSettings(name = 'Diana Q',               cast_names = ['dianaq'],                    missile_names = ['dianaqinnermissile']),
		EvadeSettings(name = 'Diana R',               cast_names = ['dianar'],                    missile_names = [])
	],
	'draven': [
		EvadeSettings(name = 'Draven E',              cast_names = ['dravendoubleshot'],          missile_names = ['dravendoubleshotmissile']),
		EvadeSettings(name = 'Draven R',              cast_names = ['dravenrcast'],               missile_names = ['dravenr'])
	],
	'drmundo': [
		EvadeSettings(name = 'Dr.Mundo Q',            cast_names = ['infectedcleavermissile'],    missile_names = ['infectedcleavermissile'])
	],
	'ekko': [
		EvadeSettings(name = 'Ekko Q',                cast_names = ['ekkoq'],                     missile_names = ['ekkoqmis']),
		EvadeSettings(name = 'Ekko W',                cast_names = ['ekkow'],                     missile_names = ['ekkowmis'])
	],
	'elise': [
		EvadeSettings(name = 'Elise Human E',         cast_names = ['elisehumane'],               missile_names = ['elisehumane'])
	],
	'evelynn': [
		EvadeSettings(name = 'Evelynn Q',             cast_names = ['evelynnq'],                  missile_names = ['evelynnq'])
	],
	'ezreal': [
		EvadeSettings(name = 'Ezreal Q',              cast_names = ['ezrealq'],                   missile_names = ['ezrealq']),
		EvadeSettings(name = 'Ezreal W',              cast_names = ['ezrealw'],                   missile_names = ['ezrealw']),
		EvadeSettings(name = 'Ezreal R',              cast_names = ['ezrealr'],                   missile_names = ['ezrealr'])
	],
	'fiddlesticks': [
		EvadeSettings(name = 'Fiddlesticks R',        cast_names = ['fiddlesticksr'],             missile_names = [])
	],
	'fizz': [
		EvadeSettings(name = 'Fizz R',                cast_names = ['fizzr'],                     missile_names = ['fizzrmissile'])
	],
	'galio': [
		EvadeSettings(name = 'Galio Q',               cast_names = ['galioq'],                    missile_names = ['galioqmissiler']),
		EvadeSettings(name = 'Galio E',               cast_names = ['galioe'],                    missile_names = []),
		EvadeSettings(name = 'Galio R',               cast_names = ['galior'],                    missile_names = [])
	],
	'gangplank': [
		EvadeSettings(name = 'GangplankR',            cast_names = ['gangplankr'],                missile_names = [])
	],
	'gnar': [		
		EvadeSettings(name = 'Gnar small Q',          cast_names = ['gnarqmissile'],              missile_names = ['gnarqmissile', 'gnarqmissilereturn']),
		EvadeSettings(name = 'Gnar big Q',            cast_names = ['gnarbigqmissile'],           missile_names = ['gnarbigqmissile'])
	],
	'gragas': [
		EvadeSettings(name = 'Graras Q',              cast_names = ['gragasq'],                   missile_names = ['gragasqmissile']),
		EvadeSettings(name = 'Gragas R',              cast_names = ['gragasr'],                   missile_names = ['gragasrboom'])
	],
	'graves': [
		EvadeSettings(name = 'Graves Q',              cast_names = ['gravesqlinespell'],          missile_names = ['gravesqlinemis', 'gravesqreturn']),
		EvadeSettings(name = 'Graves W',              cast_names = ['gravessmokegrenade'],        missile_names = ['gravessmokegrenadeboom']),
		EvadeSettings(name = 'Graves R',              cast_names = ['graveschargeshot'],          missile_names = ['graveschargeshotshot'])
	],
	'hecarim': [
		EvadeSettings(name = 'Hecarim Ult Missile',   cast_names = [],                            missile_names = ['hecarimultmissile'])
	],
	'heimerdinger': [
		EvadeSettings(name = 'Heimer W',              cast_names = ['heimerdingerw'],             missile_names = ['heimerdingerwattack2']),
		EvadeSettings(name = 'Heimer W (Ult)',        cast_names = ['heimerdingerwult'],          missile_names = ['heimerdingerwattack2ult']),
		EvadeSettings(name = 'Heimer E',              cast_names = ['heimerdingere'],             missile_names = ['heimerdingerespell']),
		EvadeSettings(name = 'Heimer E (Ult)',        cast_names = ['heimerdingereult'],          missile_names = ['heimerdingerespell_ult', 'heimerdingerespell_ult2', 'heimerdingerespell_ult3'])
	],                                                
	'illaoi': [                                       
		EvadeSettings(name = 'Illaoi Q',              cast_names = ['illaoiq'],                   missile_names = []),
		EvadeSettings(name = 'Illaoi E',              cast_names = ['illaoie'],                   missile_names = ['illaoiemis'])
	],                                                
	'irelia': [                                       
		EvadeSettings(name = 'Irelia E Stun',         cast_names = [],                            missile_names = ['ireliaeparticlemissile']),
		EvadeSettings(name = 'Irelia R',              cast_names = ['ireliar'],                   missile_names = ['ireliar'])
	],
	'ivern': [
		EvadeSettings(name = 'Ivern Q',               cast_names = ['ivernq'],                    missile_names = ['ivernq'])
	],
	'janna': [
		EvadeSettings(name = 'Janna Q Missile',       cast_names = [],                            missile_names = ['howlinggalespell' + str(i) for i in range(17)])
	],
	'jarvaniv': [
		EvadeSettings(name = 'Jarvan Q',              cast_names = ['jarvanivdragonstrike'],      missile_names = [])
	],
	'jhin': [
		EvadeSettings(name = 'Jhin W',                cast_names = ['jhinw'],                     missile_names = []),
		EvadeSettings(name = 'Jhin R',                cast_names = [],                            missile_names = ['jhinrshotmis', 'jhinrshotmis4'])
	],
	'jinx': [
		EvadeSettings(name = 'Jinx W',                cast_names = ['jhinw'],                     missile_names = ['jinxwmissile']),
		EvadeSettings(name = 'Jinx E',                cast_names = [],                            missile_names = ['jinxehit']),
		EvadeSettings(name = 'Jinx R',                cast_names = ['jinxr'],                     missile_names = ['jinxr'])
	],
	'kaisa': [
		EvadeSettings(name = 'Kaisa W',               cast_names = ['kaisaw'],                    missile_names = ['kaisaw'])
	],
	'kalista': [
		EvadeSettings(name = 'Kalista Q',             cast_names = ['kalistamysticshot'],         missile_names = ['kalistamysticshotmistrue']),
	],
	'karma': [
		EvadeSettings(name = 'Karma Q',               cast_names = ['karmaq'],                    missile_names = ['karmaqmissile', 'karmaqmissilemantra'])
	],
	'karthus': [
		EvadeSettings(name = 'Karthus Q',             cast_names = ['karthuslaywaste', 'karthuslaywastea1'], missile_names = [])
	],
	'kassadin': [
		EvadeSettings(name = 'Kassadin E',            cast_names = ['forcepulse'],                missile_names = []),
		EvadeSettings(name = 'Kassadin R',            cast_names = ['riftwalk'],                  missile_names = [])
	],
	'kayle': [
		EvadeSettings(name = 'Kayle Q',               cast_names = ['kayleq'],                    missile_names = ['kayleqmis']),
	],
	'kayn': [
		EvadeSettings(name = 'Kayn W',                cast_names = ['kaynw'],                     missile_names = [])
	],
	'khazix': [
		EvadeSettings(name = 'Khazix W',              cast_names = ['khazixw', 'khazixwlong'],    missile_names = ['khazixwmissile'])
	],
	'kled': [
		EvadeSettings(name = 'Kled Q',                cast_names = ['kledq'],                     missile_names = ['kledqmissile']),
		EvadeSettings(name = 'Kled Q (Rider)',        cast_names = ['kledriderq'],                missile_names = ['kledriderqmissile'])
	],
	'leblanc': [
		EvadeSettings(name = 'Leblanc E',             cast_names = ['leblance', 'leblancre'],     missile_names = ['leblancemissile', 'leblancremissile']),
	],
	'leesin': [
		EvadeSettings(name = 'Lee Sin Q',             cast_names = ['blindmonkqone'],             missile_names = ['blindmonkqone'])
	],
	'leona': [
		EvadeSettings(name = 'Leona E',               cast_names = ['leonazenithblade'],          missile_names = ['leonazenithblademissile']),
		EvadeSettings(name = 'Leona R',               cast_names = ['leonasolarflare'],           missile_names = [])
	],
	'lillia': [
		EvadeSettings(name = 'Lillia W',              cast_names = ['lilliaw'],                   missile_names = []),
		EvadeSettings(name = 'Lillia E',              cast_names = ['lilliae'],                   missile_names = ['lilliaerollingmissile'])
	],
	'lissandra': [
		EvadeSettings(name = 'Lissandra Q',           cast_names = ['lissandraqmissile'],         missile_names = ['lissandraqmissile'])
	],
	'lucian': [
		EvadeSettings(name = 'Lucian W',              cast_names = ['lucianw'],                   missile_names = ['lucianwmissile']),
		EvadeSettings(name = 'Lucian R',              cast_names = [],                            missile_names = ['lucianrmissile', 'lucianrmissileoffhand'])
	],
	'lulu': [
		EvadeSettings(name = 'Lulu Q',                cast_names = ['luluq'],                     missile_names = ['luluqmissile', 'luluqmissiletwo'])
	],
	'lux': [
		EvadeSettings(name = 'Lux Q',                 cast_names = ['luxlightbinding'],           missile_names = ['luxlightbindingmis']),
		EvadeSettings(name = 'Lux E',                 cast_names = ['luxlightstrikekugel'],       missile_names = ['luxlightstrikekugel']),
		EvadeSettings(name = 'Lux R',                 cast_names = ['luxmalicecannon'],           missile_names = ['luxmalicecannonmis'])
	],
	'missfortune': [
		EvadeSettings(name = 'Miss fortune E',        cast_names = ['missfortunescattershot'],    missile_names = []),
		EvadeSettings(name = 'Miss fortune R',        cast_names = ['missfortunebullettime'],     missile_names = ['missfortunebullets'])
	],
	'mordekaiser': [
		EvadeSettings(name = 'Mordekaiser Q',         cast_names = ['mordekaiserq'],              missile_names = []),
		EvadeSettings(name = 'Mordekaiser E',         cast_names = ['mordekaisere'],              missile_names = ['mordekaisere'])
	],
	'morgana': [
		EvadeSettings(name = 'Morgana Q',             cast_names = ['morganaq'],                  missile_names = ['morganaq'])
	],
	'nami': [
		EvadeSettings(name = 'Name Q',                cast_names = ['namiq'],                     missile_names = ['namiqmissile']),
		EvadeSettings(name = 'Name R',                cast_names = ['namir'],                     missile_names = ['namirmissile'])
	],
	'nautilus': [
		EvadeSettings(name = 'Nautilus Q',            cast_names = ['nautilusanchordrag'],        missile_names = ['nautilusanchordragmissile'])
	],
	'neeko': [
		EvadeSettings(name = 'Neeko Q',               cast_names = ['neekoq'],                    missile_names = ['neekoq']),
		EvadeSettings(name = 'Neeko E',               cast_names = ['neekoe'],                    missile_names = ['neekoe'])
	],
	'nidalee': [
		EvadeSettings(name = 'Nidalee Q',             cast_names = ['javelintoss'],               missile_names = ['javelintoss'])
	],
	'nocturne': [
		EvadeSettings(name = 'Nocturne Q',            cast_names = ['nocturneduskbringer'],       missile_names = ['nocturneduskbringer'])
	],
	'olaf': [
		EvadeSettings(name = 'Olaf Q',                cast_names = ['olafaxethrowcast'],          missile_names = ['olafaxethrow'])
	],
	'orianna': [
		EvadeSettings(name = 'Orianna Q',             cast_names = [],                            missile_names = ['orianaizuna'])
	],
	'pantheon': [
		EvadeSettings(name = 'Pantheon Q',            cast_names = ['pantheonqmissile'],          missile_names = ['pantheonqmissile']),
	],
	'pyke': [
		EvadeSettings(name = 'Pyke Q',                cast_names = ['pykeqrange'],                missile_names = ['pykeqrange']),
		EvadeSettings(name = 'Pyke R',                cast_names = ['pyker'],                     missile_names = [])
	],
	'qiyana': [
		EvadeSettings(name = 'Qiyana Q',              cast_names = ['quinnq'],                    missile_names = ['qiyanaq_grass', 'qiyanaq_water', 'qiyanaq_rock']),
		EvadeSettings(name = 'Qiyana R',              cast_names = ['qiyanar'],                   missile_names = ['qiyanarmis'])
	],
	'quinn': [
		EvadeSettings(name = 'Quinn Q',               cast_names = ['quinnq'],                    missile_names = ['quinnq'])
	],
	'rakan': [
		EvadeSettings(name = 'Rakan Q',               cast_names = ['rakanq'],                    missile_names = ['rakanqmis']),
		EvadeSettings(name = 'Rakan W',               cast_names = ['rakanwcast'],                missile_names = [])
	],
	'rell': [
		EvadeSettings(name = 'Rell Q',                cast_names = ['rellq'],                     missile_names = []),
		EvadeSettings(name = 'Rell R',                cast_names = ['rellr'],                     missile_names = [])
	],
	'rengar': [
		EvadeSettings(name = 'Rengar E',              cast_names = ['rengare'],                   missile_names = ['rengaremis']),
		EvadeSettings(name = 'Rengar E (Empowered)',  cast_names = ['rengareemp'],                missile_names = ['rengareempmis'])
	],
	'riven': [
		EvadeSettings(name = 'Riven R',               cast_names = ['rivenizunablade'],           missile_names = ['rivenwindslashmissileright', 'rivenwindslashmissileleft', 'rivenwindslashmissilecenter'])
	],
	'ryze': [
		EvadeSettings(name = 'Ryze Q',                cast_names = ['ryzeq'],                     missile_names = ['ryzeq'])
	],
	'samira': [
		EvadeSettings(name = 'Samira Q (Gun)',        cast_names = ['samiraqgun'],                missile_names = ['samiraqgun'])
	],
	'sejuani': [
		EvadeSettings(name = 'Sejuani R',             cast_names = ['sejuanir'],                  missile_names = ['sejuanirmissile'])
	],
	'senna': [
		EvadeSettings(name = 'Senna W',               cast_names = ['sennaw'],                    missile_names = ['sennaw']),
		EvadeSettings(name = 'Senna R',               cast_names = ['sennar'],                    missile_names = ['sennar'])
	],
	'seraphine': [
		EvadeSettings(name = 'Seraphine Q',           cast_names = ['seraphineqcast', 'seraphineqcastecho'], missile_names = ['seraphineqinitialmissile']),
		EvadeSettings(name = 'Seraphine E',           cast_names = ['seraphineecast', 'seraphineecastecho'], missile_names = ['seraphineemissile']),
		EvadeSettings(name = 'Seraphine R',           cast_names = ['seraphiner'],                missile_names = ['seraphiner'])
	],                                                
	'sett': [                                         
		EvadeSettings(name = 'Sett W',                cast_names = ['settw'],                     missile_names = [])
	],
	'shyvana': [
		EvadeSettings(name = 'Shyvana E',             cast_names = ['shyvanafireball'],           missile_names = ['shyvanafireballmissile']),
		EvadeSettings(name = 'Shyvana E (Dragon)',    cast_names = ['shyvanafireballdragon2'],    missile_names = ['shyvanafireballdragonmissile'])
	],
	'singed': [
		EvadeSettings(name = 'Singed W',              cast_names = ['megaadhesive'],              missile_names = ['singedwparticlemissile'])
	],
	'sion': [
		EvadeSettings(name = 'Sion E',                cast_names = ['sione'],                     missile_names = ['sionemissile'])
	],
	'sivir': [
		EvadeSettings(name = 'Sivir Q',               cast_names = ['sivirq'],                    missile_names = ['sivirqmissile'])
	],
	'sona': [
		EvadeSettings(name = 'Sona R',                cast_names = ['sonar'],                     missile_names = ['sonar'])
	],
	'soraka': [
		EvadeSettings(name = 'Soraka Q',              cast_names = ['sorakaq'],                   missile_names = ['sorakaqmissile']),
		EvadeSettings(name = 'Soraka E',              cast_names = ['sorakae'],                   missile_names = [])
	],
	'swain': [
		EvadeSettings(name = 'Swain Q',               cast_names = ['swainq'],                    missile_names = []),
		EvadeSettings(name = 'Swain W',               cast_names = ['swainw'],                    missile_names = []),
		EvadeSettings(name = 'Swain E',               cast_names = ['swaine'],                    missile_names = ['swaine'])
	],                                                                                            
	'sylas': [                                                                                    
		EvadeSettings(name = 'Sylas Q',               cast_names = ['sylasq'],                    missile_names = []),
		EvadeSettings(name = 'Sylas E (Jump)',        cast_names = ['sylase2'],                   missile_names = ['sylase2'])
	],                                                                                            
	'syndra': [                                                                                   
		EvadeSettings(name = 'Syndra Q',              cast_names = [],                            missile_names = ['syndraqspell']),
		EvadeSettings(name = 'Syndra E',              cast_names = ['syndrae'],                   missile_names = ['syndraemissile'])
	],
	'taliyah': [
		EvadeSettings(name = 'Taliyah Q',             cast_names = ['taliyahq'],                  missile_names = ['taliyahqmis']),
		EvadeSettings(name = 'Taliyah W',             cast_names = ['taliyahwvc'],                missile_names = []),
		EvadeSettings(name = 'Taliyah R',             cast_names = ['taliyahr'],                  missile_names = ['taliyahrmis'])
	],
	'talon': [
		EvadeSettings(name = 'Talon W',               cast_names = ['talonw'],                    missile_names = ['talonwmissileone'])
	],
	'thresh': [
		EvadeSettings(name = 'Thresh Q',              cast_names = [],                            missile_names = ['threshqmissile'])
	],
	'twistedfate': [
		EvadeSettings(name = 'Twisted Fate Q',        cast_names = ['wildcards'],                 missile_names = ['sealfatemissile'])
	],
	'twitch': [
		EvadeSettings(name = 'Twitch W',              cast_names = ['twitchvenomcask'],           missile_names = ['twitchvenomcaskmissile'])
	],
	'veigar': [
		EvadeSettings(name = 'Veigar W',              cast_names = ['veigardarkmattercastlockout'], missile_names = []),
		EvadeSettings(name = 'Veigar E',              cast_names = ['veigareventhorizon'],        missile_names = [])
	],
	'velkoz': [
		EvadeSettings(name = 'Velkoz Q',              cast_names = ['velkozq'],                   missile_names = ['velkozqmissile', 'velkozqmissilesplit']),
		EvadeSettings(name = 'Velkoz W',              cast_names = ['velkozw'],                   missile_names = ['velkozwmissile']),
		EvadeSettings(name = 'Velkoz E',              cast_names = ['velkoze'],                   missile_names = ['velkozemissile']),
		EvadeSettings(name = 'Velkoz R',              cast_names = ['velkozr'],                   missile_names = [])
	],
	'viktor': [
		EvadeSettings(name = 'Viktor W',              cast_names = ['viktorgravitonfield'],       missile_names = []),
		EvadeSettings(name = 'Viktor E',              cast_names = [],                            missile_names = ['viktordeathraymissile', 'viktoreaugmissile']),
		EvadeSettings(name = 'Viktor R',              cast_names = ['viktorchaosstorm'],          missile_names = [])
	],
	'vladimir': [
		EvadeSettings(name = 'Vladimir E',            cast_names = ['vladimire'],                 missile_names = [])
	],
	'xayah': [
		EvadeSettings(name = 'Xayah Q',               cast_names = ['xayahq'],                    missile_names = ['xayahqmissile1', 'xayahqmissile2']),
		EvadeSettings(name = 'Xayah R',               cast_names = ['xayahr'],                    missile_names = ['xayahrmissile'])
	],
	'xerath': [
		EvadeSettings(name = 'Xerath W',              cast_names = ['xerathlocuspulse'],          missile_names = []),
		EvadeSettings(name = 'Xerath E',              cast_names = ['xerathmagespear'],           missile_names = ['xerathmagespearmissile']),
		EvadeSettings(name = 'Xerath R',              cast_names = [],                            missile_names = ['xeratharcanebarrage2'])
	],
	'xinzhao': [
		EvadeSettings(name = 'XinZhao W',             cast_names = ['xinzhaow'],                  missile_names = [])
	],
	'yasuo': [
		EvadeSettings(name = 'Yasuo Q',               cast_names = ['yasuoq1', 'yasuoq2'],        missile_names = []),
		EvadeSettings(name = 'Yasuo Q (Tornado)',     cast_names = ['yasuoq3'],                   missile_names = ['yasuoq3mis'])
	],
	'yone': [
		EvadeSettings(name = 'Yone Q',                cast_names = ['yoneq'],                     missile_names = []),
		EvadeSettings(name = 'Yone Q (Jump)',         cast_names = ['yoneq3'],                    missile_names = ['yoneq3']),
		EvadeSettings(name = 'Yone W',                cast_names = ['yonew'],                     missile_names = []),
		EvadeSettings(name = 'Yone R',                cast_names = ['yoner'],                     missile_names = ['yoner'])
	],
	'yuumi': [
		EvadeSettings(name = 'Yuumi R',               cast_names = ['yuumir'],                    missile_names = ['yuumirmissile'])
	],
	'zac': [
		EvadeSettings(name = 'Zac Q',                 cast_names = ['zacq'],                      missile_names = ['zedqmissile']),
		EvadeSettings(name = 'Zac E',                 cast_names = ['zace'],                      missile_names = [])
	],
	'zed': [
		EvadeSettings(name = 'Zed Q',                 cast_names = ['zedq'],                      missile_names = ['zedqmissile'])
	],
	'ziggs': [
		EvadeSettings(name = 'Ziggs Q',               cast_names = ['ziggsq'],                    missile_names = ['ziggsqspell', 'ziggsqspell2', 'ziggsqspell3']),
		EvadeSettings(name = 'Ziggs W',               cast_names = ['ziggsw'],                    missile_names = ['ziggsw']),
		EvadeSettings(name = 'Ziggs E',               cast_names = ['ziggse'],                    missile_names = ['ziggse2']),
		EvadeSettings(name = 'Ziggs R',               cast_names = ['ziggsr'],                    missile_names = ['ziggsrboom', 'ziggsrboomlong', 'ziggsrboommedium', 'ziggsrboomextralong'])
	],
	'zilean': [
		EvadeSettings(name = 'Zilean Q',              cast_names = ['zileanq'],                   missile_names = ['zileanqmissile'])
	],
	'zoe': [
		EvadeSettings(name = 'Zoe Q',                 cast_names = ['zoeq'],                      missile_names = ['zoeqmis2', 'zoeqmissile']),
		EvadeSettings(name = 'Zoe E',                 cast_names = ['zoee'],                      missile_names = ['zoee'])
	],
	'zyra': [
		EvadeSettings(name = 'Zyra E',                cast_names = ['zyrae'],                     missile_names = ['zyrae']),
		EvadeSettings(name = 'Zyra R',                cast_names = ['zyrar'],                     missile_names = [])
	]
}

NameToSettings = {}

#EvadeSettings(name = '',  cast_names = [''], missile_names = [''])

def valkyrie_menu(ctx) :		 
	global always_evade, enabled, extra_evade_length
	ui = ctx.ui	

	ui.text('Champions settings', Col.Purple)
	for champ in ctx.champs.enemy_to(ctx.player).get():
		cname = champ.name
		if cname in Settings:
			ui.image(cname + '_square', Vec2(16, 16))
			ui.sameline()
			if ui.beginmenu(cname):
				for s in Settings[cname]:
					ui.pushid(id(s))
					s.ui(ui)
					ui.popid()
					
				ui.endmenu()

	ui.separator()
	ui.text('Global settings', Col.Purple)
	enabled            = ui.checkbox('Enabled', enabled)
	always_evade       = ui.checkbox('Always try to dodge', always_evade)
	#extra_evade_length = ui.sliderfloat('Extra evade distance', extra_evade_length, 50.0, 200.0)
	
	ui.separator()
	if ui.treenode('Some notes'):
		ui.text('Most of these will be fixed')
		ui.text('1. Orbwalker + Evade not fully compatible with all spells')
		ui.text('2. Currently not evading cones')
		ui.text('3. Currently only sidesteps skillshots no dahes/flash')
		ui.text('4. Since it simulates clicks it might click an minion. Dont tank the wave and expect evades.')
		ui.text('5. Doesnt check for walls so it might evade in walls')
		ui.text('5. Following champs not supported (cause i dont have them): ')
		for name in NotSupportedYet:
			ui.image(name + '_square', Vec2(20, 20))
			ui.sameline()
		ui.treepop()
		
def valkyrie_on_load(ctx) :	 
	global always_evade, enabled, extra_evade_length
	global Settings, NameToSettings
	cfg = ctx.cfg
	
	enabled      = cfg.get_bool('_enabled', enabled)
	always_evade = cfg.get_bool('_always_evade', always_evade)
	#extra_evade_length = cfg.get_float('_extra_evade_length', extra_evade_length)
	for champ, settings in Settings.items():
		for i, default_setting in enumerate(settings):
			setting = EvadeSettings.from_str(cfg.get_str(default_setting.name, str(default_setting)))
			settings[i] = setting
			
			for cast in setting.cast_names:
				NameToSettings[cast] = setting
			for mis in setting.missile_names:
				NameToSettings[mis] = setting
	
	
def valkyrie_on_save(ctx) :	 
	cfg = ctx.cfg
	
	cfg.set_bool('_enabled', enabled)
	cfg.set_bool('_always_evade', always_evade)
	#cfg.set_float('_extra_evade_length', extra_evade_length)
	for champ, settings in Settings.items():
		for setting in settings:
			cfg.set_str(setting.name, str(setting))
	
def perpedincular_dist_to_segment(p, s1, s2):
	px = s2.x - s1.x
	py = s2.y - s1.y

	norm = px*px + py*py

	u =  ((p.x - s1.x) * px + (p.y - s1.y) * py) / float(norm)

	if u > 1:
		u = 1
	elif u < 0:
		u = 0

	x = s1.x + u * px
	y = s1.y + u * py

	dx = x - p.x
	dy = y - p.y

	return (dx*dx + dy*dy)**.5
	
def check_sidestepable(ctx, col, distance, player):
	if always_evade:
		return True
		
	time_evade = distance/player.move_speed
	time_impact = col.time_until_impact + extra_evade_length / player.move_speed
	
	if time_evade > time_impact:
		#ctx.pill("Unavoidable", Col.Black, Col.Red)
		return False
	
	ctx.info(f"{col.spell.name} : evade in {time_evade:.2f}, impact in {time_impact:.2f}")
	return True
	
def get_area_evade_point(ctx, player, col):
	
	evade_dir = (player.pos - col.spell.end_pos).normalize()
	threshold = extra_evade_length + col.spell.static.cast_radius + player.static.gameplay_radius
	dist_to_center = col.spell.end_pos.distance(player.pos)
	if dist_to_center > threshold:
		
		return player.pos
		
	distance  = threshold - dist_to_center
		
	return player.pos + (evade_dir * distance)
	
def get_line_evade_point(ctx, player, col):
	
	# Make a line for checking on what side the unit is on
	unit_curr_pos = player.pos
	
	line_dir = Vec2(col.spell.dir.x, col.spell.dir.z)
	line_end = (line_dir * 1000) + col.spell_pos
	line_start = (line_dir * -1000) + col.spell_pos
	determinant = ((line_end.x - line_start.x) * (unit_curr_pos.z - line_start.y)) - ((line_end.y - line_start.y) * (unit_curr_pos.x - line_start.x))
	
	spell_dir = col.spell.dir.clone()
	spell_dir.y = 0.0
	
	pdist = perpedincular_dist_to_segment(Vec2(unit_curr_pos.x, unit_curr_pos.z), line_start, line_end)
	dist_threshold = extra_evade_length + col.spell.static.width + col.unit.static.gameplay_radius
	if pdist > dist_threshold:
		return unit_curr_pos
	
	distance = dist_threshold - pdist
		
	# Check if player is on left or right of the line
	if determinant < 0.0:
		return unit_curr_pos + (spell_dir.rotate_y(RADIANS_90_DEG) * distance)
	else:
		return unit_curr_pos + (spell_dir.rotate_y(-RADIANS_90_DEG) * distance)
	
	return None
	
	
def find_collision_to_evade(ctx, collisions):
	
	best_col = None
	best_col_prio = 0
	for col in collisions:
		spellname = col.spell.name
		setting = NameToSettings.get(spellname, None)
		if not setting:
			continue
		
		if not setting.evade:
			continue
			
		# Check if its a spell casting
		if not setting.evade_cast and col.spell.remaining > 0.0:
			continue
		
		# Get collision with highest prio
		if setting.priority >= best_col_prio:
			best_col_prio = setting.priority
			best_col = col
	
	# Check if we are already evading a more important spell
	if EvadeFlags.EvadeEndTime > time.time() and EvadeFlags.CurrentEvadePriority >= best_col_prio:
		return None
	
	EvadeFlags.CurrentEvadePriority = best_col_prio
	return best_col
	
def move(ctx, timenow):
	global last_moved
	
	if timenow - last_moved > move_cooldown:
		ctx.move(EvadeFlags.EvadePoint)
		last_moved = timenow
	
def valkyrie_exec(ctx) :	     
	if not enabled:
		return
	
	now = time.time()
	if now < EvadeFlags.EvadeEndTime:
		ctx.pill("Evading", Col.Black, Col.Yellow)
		move(ctx, now)
	
	ctx.pill("Evade", Col.Black, Col.Green)
	
	player = ctx.player
	collisions = ctx.collisions_for(player)
	
	if len(collisions) == 0:
		return
		
	col = find_collision_to_evade(ctx, collisions)
	if not col:
		return
		
	evade_point = None
	if col.spell.static.has_flag(Spell.TypeLine):
		evade_point = get_line_evade_point(ctx, player, col)
	elif col.spell.static.has_flag(Spell.TypeArea):
		evade_point = get_area_evade_point(ctx, player, col)
	
	if not check_sidestepable(ctx, col, player.pos.distance(evade_point), player):
		return None

	if not evade_point:
		return
	
	EvadeFlags.EvadeEndTime = now + (evade_point.distance(player.pos) / player.move_speed) 
	EvadeFlags.EvadePoint = evade_point
	move(ctx, now)
	
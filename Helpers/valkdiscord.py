import discord
import boto3
import threading
import json
import time
from sys import argv
from pprint import pprint
from datetime import datetime

SUB_EXPIRATION_THRESHOLD = 1296000.0 # 15 DAYS
DISCORD_TOKEN = 'ODMyOTk3MzQ4MDMxNTk0NTA4.YHr7PA.48nwQqHXQurU1UIKHSFVvXAhgRc'
AWS_ACCESS_KEY = 'AKIAU6GDVTT2BO5OFOE5'
AWS_SECRET_KEY = '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'

VALKY_REQUEST_JSON = json.dumps({
	"operation": "list-users",
	"operation-params": {
		"name": "TeamValkyrie",
		"pass": "88saidu33",
		"hardware": {
			"cpu": "178BFBFF00800F11",
			"gpu": "PCI\\VEN_10DE&DEV_1C02&SUBSYS_1C0210DE&REV_A1\\4&1C3D25BB&0&0019",
			"ram": "6E0F0B74",
			"system": "DESKTOP-HS3JVV9"
		},
	}
}).encode()

NEW_MEMBER_COPY_PASTA = '''
:fire: Thank you for purchasing valkyrie ! :fire:
Go to :arrow_double_down: download section and read & follow all the instruction there
You can make an account in loader using this code **{}**
If you bought using crypto use the code you bought to extend subscription
If you have issues contact accountjpeg@7110 :)

Have a nice day ! :heart:
'''

# Make aws lambda client
lambda_client = boto3.client(
    'lambda',
    aws_access_key_id=AWS_ACCESS_KEY,
    aws_secret_access_key=AWS_SECRET_KEY,
	region_name = 'eu-north-1'
)

# Make discord bot client
intents = discord.Intents.none()
intents.reactions = True
intents.members = True
intents.guilds = True

discord_client = discord.Client(intents = intents)
role_member = None
log_channel = None

async def log_message(message):
	#print(message)
	await log_channel.send(message)
	
def has_member_role(member):
	for role in member.roles:
		if role.name == 'Member':
			return True
	return False

def get_valk_users_by_discord():
	''' Returns a dict with valkyrie users, key is discord '''
	lambda_response = lambda_client.invoke(
		FunctionName = 'valkyrie-api',
		Payload = VALKY_REQUEST_JSON
	)

	valky_users = json.loads(lambda_response['Payload'].read().decode())['result']
	return {vuser['discord']: vuser for vuser in valky_users}

async def add_membership(member):
	await member.add_roles(role_member)
	await log_message(f"Promoted <@{member.id}> to Member")

async def remove_membership(member):
	await member.remove_roles(role_member)
	await log_message(f"Demoting <@{member.id}> (sub expired for more than 15 days)")

async def sync_roles_with_valkyrie():
	
	# Discord users
	discord_members = discord_client.get_all_members()
	discord_members = {f'{user.name}#{user.discriminator}': user for user in discord_members}
	
	# Get valkyrie users
	valky_users = get_valk_users_by_discord()

	for discord_name, duser in discord_members.items():
		
		vuser = valky_users.get(discord_name, None)
		if vuser == None:
			print(f'User on discord but not on valkyrie {discord_name}')
			continue
		
		is_discord_member = has_member_role(duser)
		valid_sub = ((vuser['expiry'] - time.time()) > -SUB_EXPIRATION_THRESHOLD)
		if is_discord_member and not valid_sub:
			print(f'Removing membership for {discord_name}')
			await remove_membership(duser)
		elif not is_discord_member and valid_sub:
			print(f'Adding membership for {discord_name}')
			await add_membership(duser)

async def count_members():
	discord_members = discord_client.get_all_members()
	
	count = 0
	for member in discord_members:
		if has_member_role(member):
			count += 1
			
	print(f'Number of customers: {count}')
	
async def prune_lurkers():
	''' Kicks people that have been lurking without buying sub for 7 days '''
	
	discord_members = discord_client.get_all_members()
	discord_members = {f'{user.name}#{user.discriminator}': user for user in discord_members}
	valk_users = get_valk_users_by_discord()
	
	count = 0
	for nametag, member in discord_members.items():
		if len(member.roles) > 1 or nametag in valk_users:
			continue
			
		lurking_time = datetime.now() - member.joined_at
		if lurking_time.days > 5:
			count += 1
			print('Kicking ' + nametag)
			await member.kick(reason = 'Lurking too many days without buying a subscription. Feel free to join back if you want to the product')
			await log_message(f'Kicked <@{member.id}> for lurking too much')
	
	if count > 0:
		log_message(f'Kicked {count} lurkers')
	
async def add_new_customer(nametag, mode):
	''' @param nametag: must be a string of the form name#1234 '''

	# Find member
	discord_members = discord_client.get_all_members()
	discord_members = {f'{user.name}#{user.discriminator}': user for user in discord_members}
	
	member = discord_members.get(nametag, None)
	if not member:
		print(f'User {nametag} not found')
		return
	
	if has_member_role(member):
		print('User already member')
		return
	
	# Read codes file and extract code
	codes = None
	with open('codes.json', 'r') as f:
		codes = json.loads(f.read())
	
	if mode not in codes:
		print('Unknown mode ' + mode)
		return
		
	code_list = codes[mode]
	if len(code_list) == 0:
		print('No more codes')
		return
	
	# Take code and create DM
	code = code_list[-1]
	dm = NEW_MEMBER_COPY_PASTA.format(code)
	
	# Send DM
	await add_membership(member)
	await member.send(dm)
	print('Send DM to ' + nametag)
	
	# Update codes file
	del code_list[-1]
	with open('codes.json', 'w') as f:
		f.write(json.dumps(codes, indent=4))
		
	
execution_modes = {
	'sync' : sync_roles_with_valkyrie,
	'count': count_members,
	'prune': prune_lurkers,
	'add'  : add_new_customer
}

@discord_client.event
async def on_ready():
	global log_channel, role_member
	log_channel = discord_client.get_channel(832979903589908482)
	if log_channel == None:
		raise Exception("No log channel found")
		
	guild = await discord_client.fetch_guild(832979903574048819)
	role_member = guild.get_role(832979903574048824)
	
	mode = execution_modes.get(argv[1], None)
	if not mode:
		print('Unknown execution mode ' + argv[1])
		return
	
	await mode(*argv[2:])
	await discord_client.close()
	
discord_client.run(DISCORD_TOKEN)
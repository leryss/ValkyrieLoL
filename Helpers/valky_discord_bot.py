import discord
import boto3
import threading
import json
import time
from pprint import pprint

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
	lambda_response = lambda_client.invoke(
		FunctionName = 'valkyrie-api',
		Payload = VALKY_REQUEST_JSON
	)

	valky_users = json.loads(lambda_response['Payload'].read().decode())['result']
	valky_users = {vuser['discord']: vuser for vuser in valky_users}

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
			
@discord_client.event
async def on_ready():
	global log_channel, role_member
	log_channel = discord_client.get_channel(832979903589908482)
	guild = await discord_client.fetch_guild(832979903574048819)
	role_member = guild.get_role(832979903574048824)
	
	if log_channel == None:
		raise Exception("No log channel found")

	while(True):
		print('Starting Role Synchronizer')
		await sync_roles_with_valkyrie()
		print('Role synchronization over. Sleeping...')
		time.sleep(3600)
	
discord_client.run(DISCORD_TOKEN)
import boto3
import time, string, random, re
from boto3.dynamodb.conditions import Key
from pprint import pprint
from decimal import Decimal
from math import inf

INVITE_MODE_CREATE    = 0
INVITE_MODE_EXTEND    = 1

# User permissions
PERMISSION_USER       = 0
PERMISSION_TESTER     = 1
PERMISSION_ADMIN      = 2
PERMISSION_SUPERADMIN = 3

# Script submission status
SUBMISSION_PENDING    = 0
SUBMISSION_APPROVED   = 1
SUBMISSION_DENIED     = 2

# Some cached tables
DynamoDB     = boto3.resource('dynamodb')
UsersTable   = DynamoDB.Table('valkyrie-users')
InvitesTable = DynamoDB.Table('valkyrie-invites')
ScriptsTable = DynamoDB.Table('valkyrie-scripts')

class ParamInt:
    
    def __init__(self, name, bounds = (-inf, inf)):
        self.name = name
        self.bounds = bounds
    
    def check(self, i):
        if type(i) is not int:
            return 'Not integer'
        if i < self.bounds[0] or i > self.bounds[1]:
            return 'Not within bounds'
        return None

class ParamBool:
    
    def __init__(self, name):
        self.name = name
        
    def check(self, b):
        if type(b) is not bool:
            return 'Is not boolean'
        return None
        
class ParamFloat:
    
    def __init__(self, name, bounds = (-float("inf"), float("inf"))):
        self.name = name
        self.bounds = bounds
        
    def check(self, f):
        if type(f) is not float:
            return f'Not decimal ({f})'
        if f < self.bounds[0] or f > self.bounds[1]:
            return f'Float outside bounds'
            
        return None

class ParamStr:
    
    def __init__(self, name = '', minlen = 0, maxlen = 0, regex = re.compile('\w*')):
        self.name   = name
        self.minlen = minlen
        self.maxlen = maxlen
        self.regex  = regex
        
    def check(self, s):
        if type(s) is not str:
            return 'Is not str'
        
        if len(s) < self.minlen or len(s) > self.maxlen:
            return f'Length {len(s)} is not between [{self.minlen}, {self.maxlen}]'
            
        if self.regex and not self.regex.match(s):
            return f'Doesnt match pattern {self.regex.pattern}'
            
        return None
        
class ParamDict:
    
    def __init__(self, name = '', checkers = {}):
        self.checkers = checkers
        self.name = name
    
    def check(self, h):
        if type(h) is not dict:
            return 'Not dict'
        
        for checker in self.checkers:
            if checker.name not in h:
                return f'Missing {checker.name}'
            chk = checker.check(h[checker.name])
            if chk is not None:
                return f'{checker.name}->{chk}'
                
        return None
        
# API Input parameter validators
PARAM_NAME               = ParamStr('name', 5, 30)
PARAM_SUMMONER_NAME      = ParamStr('summoner_name', 3, 30, re.compile(".+"))
PARAM_TARGET             = ParamStr('target', 5, 30)
PARAM_PASS               = ParamStr('pass', 5, 30)
PARAM_DISCORD            = ParamStr('discord', 5, 30, re.compile("[^#]+#\d\d\d\d"))
PARAM_LOCKED             = ParamBool('locked')
PARAM_EXPIRY             = ParamInt('expiry')
PARAM_TIMESTAMP          = ParamInt('timestamp')
PARAM_DAYS               = ParamInt('days')
PARAM_LEVEL              = ParamInt('level')
 
PARAM_NUM_INVITES        = ParamInt('num-invites')
PARAM_INVITE_CODE        = ParamStr('code', 5, 50)
PARAM_INVITE_MODE        = ParamInt('mode')
PARAM_INVITE             = ParamDict('invite', [PARAM_DAYS, PARAM_LEVEL, PARAM_INVITE_MODE, PARAM_INVITE_CODE])
 
PARAM_HARDWARE           = ParamDict('hardware', [ParamStr('cpu', 5, 30, None), ParamStr('gpu', 5, 200, None), ParamStr('ram', 0, 30, None), ParamStr('system', 1, 50, None)]) 
PARAM_TARGET_INFO        = ParamDict('target-info', [PARAM_DISCORD, PARAM_HARDWARE, PARAM_LOCKED, PARAM_EXPIRY, PARAM_LEVEL])
       
PARAM_ID                 = ParamStr('id', 5, 30)
PARAM_DESCRIPTION        = ParamStr('description', 20, 150)
PARAM_CHAMPION           = ParamStr('champion', 2, 15)
PARAM_CODE               = ParamStr('code', 100, 50000)
PARAM_DENY_REASON        = ParamStr('deny_reason', 0, 250)
PARAM_STATUS             = ParamInt('status')
PARAM_RATING             = ParamInt('rating', (1, 5))
PARAM_SCRIPT             = ParamDict('script', [PARAM_ID, PARAM_NAME, PARAM_DESCRIPTION, PARAM_CHAMPION])
PARAM_SCRIPT_SUBMISSION  = ParamDict('script-submission', [PARAM_SCRIPT, PARAM_STATUS, PARAM_DENY_REASON])
PARAM_SESSION_INFO       = ParamDict('session-info', [PARAM_SUMMONER_NAME, PARAM_TIMESTAMP])

# Utilities
def query_by_key(tbl, key_val, key_name):
    query = tbl.query(KeyConditionExpression=Key(key_name).eq(key_val))
    if len(query['Items']) == 0:
        return None
        
    return query['Items'][0]
    
def extract_user_response(user):
    return {
        'name'          : user['name'],
        'locked'        : user['locked'],
        'hardware'      : user.get('hardware', {}),
        'discord'       : user['discord'],
        'expiry'        : float(user['expiry']),
        'level'         : float(user['level']),
        'reset_hardware': user['reset_hardware']
    }
    
def extract_params(*args, **kwargs):
    '''
        Extracts params and validates them
    '''
    param_list = args[0]
    def verify(func):
        def verify_inner(*args, **kwargs):
            raw = args[0]
            result_params = []
            for param in param_list:
                if param.name not in raw:
                    return error(f"Parameter {param.name} missing")
                    
                chk = param.check(raw[param.name])
                if chk is not None:
                    return error(f'Invalid `{param.name}`: {chk}')
                result_params.append(raw[param.name])
                
            return func(*result_params)
        return verify_inner
        
    return verify

def verify_identity(func):
    '''
        Verifies the identity of the caller.
    '''
    def verify(params):
        try:
            name      = params['name']
            pass_hash = params['pass']
            hardware  = params['hardware']
        except:
            return error('Bad request! Missing name, pass or hardware!')
            
        user = query_by_key(UsersTable, name, 'name')
        if user == None:
            return error(f"No user {name} found")
        
        if user['expiry'] < Decimal(time.time()):
            return error('Subscription expired')
            
        if user['locked']:
            return error('User locked')
        
        if user['pass'] != pass_hash:
            return error('Wrong password')
            
        # If we dont have hardware info in DB that means we got a HWID reset
        if user['reset_hardware']:
            UsersTable.update_item(
                Key={'name': name}, 
                UpdateExpression="set hardware=:h, reset_hardware=:r",
                ExpressionAttributeValues={':h': hardware, ':r': False}    
            )
            print('HWID Reset was performed')
        elif user['hardware'] != hardware:
            return error("HWID Mismatch")
        
        return func(params)
    return verify
    
def check_permissions(*args, **kwargs):
    '''
        Verifies that the user 'name' has permissions to modify the user 'target'
    '''
    
    at_least   = kwargs.get('AtLeast', PERMISSION_USER)
    for_target = kwargs.get('BetterThanTarget', False)
    
    def verify(func):
        def verify_inner(*args, **kwargs):
            params = args[0]
            try:
                name = params['name']
                target_name = params['target'] if for_target else None
            except:
                return error("Bad request! Not enough arguments to check permissions!")
            
            # Always allow user to perform operations on himself
            if for_target and name == target_name:
                return func(*args, **kwargs)
                
            caller = query_by_key(UsersTable, name, 'name')
            target = query_by_key(UsersTable, target_name, 'name') if for_target else None
            
            if caller == None:
                return error("Caller user doesnt exist")
            if for_target and target == None:
                return error("Target user doesnt exist")
                
            if caller['level'] < at_least or (for_target and (caller['level'] <= target['level'])):
                return error("Not enough permissions!")
        
            return func(*args, **kwargs)
        return verify_inner
    return verify
    
# User operation
@extract_params([PARAM_NAME, PARAM_PASS, PARAM_DISCORD, PARAM_HARDWARE, PARAM_INVITE_CODE])
def create_account(name, pass_hash, discord, hardware, invite_code):
    '''
        Creates an account from a invite link. Does not require authorization before hand
    '''
    
    dbcode = query_by_key(InvitesTable, invite_code, 'code')
    if not dbcode:
        return error('Invalid code')
    if dbcode['mode'] != INVITE_MODE_CREATE:
        return error('Code is not for account creation')
    subscription_days = dbcode['days']
    level = dbcode['level']

    user = query_by_key(UsersTable, name, 'name')
    if user != None:
        return error(f"User {name} already exists")
        
    new_user = {
        "name"           : name,
        "pass"           : pass_hash,
        "discord"        : discord,
        "hardware"       : hardware,
        "locked"         : False,
        "level"          : level,
        "reset_hardware" : True,
        "expiry"         : Decimal(time.time()) + (subscription_days * Decimal(24.0*60.0*60.0)),
    }
    UsersTable.put_item(Item=new_user)
    
    InvitesTable.delete_item(Key={'code': invite_code})
    return success(extract_user_response(new_user))

@verify_identity
@check_permissions(AtLeast=PERMISSION_ADMIN, BetterThanTarget=True)
@extract_params([PARAM_TARGET])
def delete_account(target):
    UsersTable.delete_item(Key={"name": target})
    
    return success("Successfully deleted user " + target)

@verify_identity
@check_permissions(AtLeast=PERMISSION_ADMIN, BetterThanTarget=True)
@extract_params([PARAM_NAME, PARAM_TARGET, PARAM_TARGET_INFO])
def update_user(caller, target, user):
    
    caller_user = query_by_key(UsersTable, caller, 'name')
    if user['level'] >= caller_user['level']:
        return error("Insufficient permissions")
        
    try:
        new_user = query_by_key(UsersTable, target, 'name')
        new_user['discord']  = user['discord']
        new_user['hardware'] = user['hardware']
        new_user['locked']   = user['locked']
        new_user['expiry']   = user['expiry']
        new_user['level']    = user['level']
        new_user['reset_hardware'] = user['reset_hardware']
    except:
        return error("Invalid user format")
        
    UsersTable.put_item(Item=new_user)
    
    return success(extract_user_response(new_user))

@verify_identity
@check_permissions(BetterThanTarget=True)
@extract_params([PARAM_TARGET])
def get_user(target):
    #return error("Loggin in is frozen until Darkrai pays the dev and gets his shit together")
    user = query_by_key(UsersTable, target, 'name')
    
    return success(extract_user_response(user))

@verify_identity
@check_permissions(AtLeast=PERMISSION_ADMIN)
def list_users(params):
    query = UsersTable.scan()
    
    users = []
    for user in query['Items']:
        users.append(extract_user_response(user))
        
    return success(users)
    
@verify_identity
def list_scripts(params):
    query = ScriptsTable.scan()
    
    return success(query['Items'])

@verify_identity
@extract_params([PARAM_ID])
def get_script_code(script_id):
    
    tbl = DynamoDB.Table('valkyrie-scripts-code')
    script = query_by_key(tbl, script_id, 'id')
    if script == None:
        return error("Script code no longer exists")
        
    return success(script['code'])

@verify_identity
@check_permissions(AtLeast=PERMISSION_ADMIN)
def get_all_submissions(params):
    
    tbl = DynamoDB.Table('valkyrie-script-submissions')
    scan = tbl.scan()
    
    submissions = []
    for entry in scan['Items']:
        submissions += list(entry['submissions'].values())
        
    return success(submissions)

@verify_identity
@extract_params([PARAM_TARGET])
def get_submissions_by_name(target):
    
    tbl = DynamoDB.Table('valkyrie-script-submissions')
    entry = query_by_key(tbl, target, 'user')
    
    submissions = list(entry['submissions'].values()) if entry else []
    return success(submissions)

@verify_identity
@check_permissions(AtLeast=PERMISSION_ADMIN)
@extract_params([PARAM_SCRIPT_SUBMISSION])
def update_submission(submission):
    
    script = submission['script']
    tbl_submissions = DynamoDB.Table('valkyrie-script-submissions')
    submission_entry = query_by_key(tbl_submissions, script['author'], 'user')
    
    if not submission_entry:
        return error(f"User {script['author']} has no submissions")
    
    script_id = script['id']
    db_submission = submission_entry['submissions'].get(script_id)
    if not db_submission:
        return error('Submission no longer exists')
    
    # Update submission entry
    submission_entry['submissions'][script_id] = submission
    tbl_submissions.put_item(Item=submission_entry)
    
    if submission['status'] != SUBMISSION_APPROVED:
        return success(None)
    
    # Update script
    db_script = query_by_key(ScriptsTable, script_id, 'id')
    if not db_script:
        db_script = script
    else:
        db_script['name'] = script['name']
        db_script['champion'] = script['champion']
        db_script['description'] = script['description']
    db_script['last_updated'] = Decimal(time.time())
    
    ScriptsTable.put_item(Item = db_script)
    
    # Update code entry
    tbl_code = DynamoDB.Table('valkyrie-scripts-code')
    code_entry = query_by_key(tbl_code, script_id + '_submission', 'id')
    if not code_entry:
        return error('Submission is corrupted: Missing code entry')
    code_entry['id'] = script_id
    tbl_code.put_item(Item=code_entry)
    
    return success(None)

@verify_identity
@extract_params([PARAM_NAME, PARAM_SCRIPT, PARAM_CODE])
def submit_script(caller_name, script, code):
    ''' Submits a script update/upload. Admins are trusted therfore their submission is directly approved '''
    
    # Get script from db and check ownership
    tbl_submissions = DynamoDB.Table('valkyrie-script-submissions')
    db_script = query_by_key(ScriptsTable, caller_name, 'id')
    if db_script and db_script['author'] != caller_name:
        return error('Cant update what is not yours')
    
    # Get submissions entry for user
    entry = query_by_key(tbl_submissions, caller_name, 'user')
    if not entry:
        entry = { 'user': caller_name, 'submissions': {} }

    submission = {}
    submission['script'] = script
    submission['status'] = SUBMISSION_PENDING
    submission['deny_reason'] = ''
    
    script_id = script['id']
    if 'submission' in script_id:
        return error('Word `submission` is forbidden in script id')
        
    entry['submissions'][script_id] = submission
    if sum([1 if s['status'] == SUBMISSION_PENDING else 0 for s in entry['submissions'].values()]) > 5:
        return error('You have too many pending submissions')
        
    caller = query_by_key(UsersTable, caller_name, 'name')
    
    id_code = script_id + "_submission"
    tbl_code = DynamoDB.Table('valkyrie-scripts-code')
    db_code = query_by_key(tbl_code, id_code, 'id')
    if db_code and db_code['author'] != script['author']:
        return error(f'Someone else has already made a submission for {script_id}.py')
    tbl_code.put_item(Item={'id': id_code, 'code': code, 'author': script['author']})

    tbl_submissions.put_item(Item=entry)
    
    return success([submission])

@verify_identity
@check_permissions(AtLeast=PERMISSION_ADMIN)
@extract_params([PARAM_NAME, PARAM_INVITE, PARAM_NUM_INVITES])
def generate_invite(caller, invite, num_invites):
    ''' Generates a random invite link and adds it to the database '''
    
    level = invite['level']
    days = invite['days']
    mode = invite['mode']
    
    caller = query_by_key(UsersTable, caller, 'name')
    if level >= caller['level']:
        return error("Insufficient permissions")
    
    items = []
    for i in range(num_invites):
        code = ''.join([random.choice(string.ascii_letters) for i in range(20)])
        code = f"{'create' if mode == INVITE_MODE_CREATE else 'extend'}-{days}-{level}-{code}"
        items.append({
            'code' : code,
            'days' : days,
            'level': level,
            'mode' : mode
        })
    
    for item in items:
        InvitesTable.put_item(Item = item)
    
    return success(items)

@verify_identity
@extract_params([PARAM_NAME, PARAM_SESSION_INFO])
def log_session(username, session):
    session['user'] = username
    tbl = DynamoDB.Table('valkyrie-sessions')
    tbl.put_item(
        Item = session
    )
    
    return success('')

@verify_identity
@extract_params([PARAM_ID, PARAM_NAME, PARAM_RATING])
def rate_script(script_id, caller, rating):
    
    rating = Decimal(rating)
    
    # Get script entry
    script_entry = query_by_key(ScriptsTable, script_id, 'id')
    if not script_entry:
        return error(f'No script found with specified id {script_id}')
        
    # Update stats
    ScriptStatsTable = DynamoDB.Table('valkyrie-script-stats')
    stats = query_by_key(ScriptStatsTable, script_id, 'script_id')
    if stats == None:
        stats = { 'script_id': script_id, 'ratings': {} }
    
    stats['ratings'][caller] = rating
    ScriptStatsTable.put_item(Item = stats)
    
    # Update script entry
    rating_dict = {
        'average_rating': sum([rating for rating in stats['ratings'].values()]) / len(stats['ratings']),
        'num_ratings': len(stats['ratings'])
    }
    script_entry.update(rating_dict)
    ScriptsTable.put_item(Item = script_entry)
    
    return success(rating_dict)
    
@extract_params([PARAM_NAME, PARAM_INVITE_CODE])
def extend_sub(name, code):
    ''' Extends subscription of a user using a subscription code '''
    
    dbcode = query_by_key(InvitesTable, code, 'code')
    if not dbcode:
        return error('Invalid code')
    if dbcode['mode'] != INVITE_MODE_EXTEND:
        return error('Code is not for subscription extension')
    
    dbuser = query_by_key(UsersTable, name, 'name')
    if not dbuser:
        return error(f"User {name} doesnt exists")
    
    now = time.time()
    if dbuser['expiry'] < now:
        dbuser['expiry'] = Decimal(now)
    dbuser['expiry'] += Decimal(24.0*60.0*60.0) * dbcode['days']
    
    UsersTable.put_item(Item=dbuser)
    InvitesTable.delete_item(Key={'code': code})
    return success(str(dbuser['expiry']))

def success(msg):
    return {
        'code': 200,
        'result': msg
    }
    
def error(msg):
    return {
        'code': 400,
        'error': msg
    }

guest_operations = {
    'create-account' : create_account,
    'extend-sub'     : extend_sub
}
    
user_operations = {
    # Any user
    'get-user'         : get_user,
    'list-scripts'     : list_scripts,
    'get-script-code'  : get_script_code,
    'submit-script'    : submit_script,
    'submissions-for'  : get_submissions_by_name,
    'log-session'      : log_session,
    'rate-script'      : rate_script,
    
     
    # Admin only
    'delete-account'  : delete_account,
    'update-user'     : update_user,
    'list-users'      : list_users,
    'generate-invite' : generate_invite,
    'list-submissions' : get_all_submissions,
    'update-submission': update_submission
}

def lambda_handler(event, context):
    
    if 'operation' not in event or 'operation-params' not in event:
        return error("Bad request! Missing essential parameters!")
        
    op = event['operation']
    params = event['operation-params']
    
    if op in guest_operations:
        return guest_operations[op](params)
    else:
        if op not in user_operations:
            return error("Unknown operation")
        return user_operations[op](params)
        
    return error('Unknown operation provided!')

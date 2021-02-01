import sys, urllib.request, re, time, os
from pprint import pprint

headers = {}
headers['User-Agent'] = "Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.27 Safari/537.17"
pattern_item = '<a href="[\w\.]+/?" title="[\w\.]+">([\w\.]+)/?</a>'

def download_jsons_from(url, save_folder):
    global pattern_item
    
    if not os.path.exists(save_folder):
        os.mkdir(save_folder)
        
    print('Requesting: ' + url)
    req = urllib.request.Request(url, headers = headers)
    page = urllib.request.urlopen(req).read().decode('utf-8')
    
    matches = re.findall(pattern_item, page)
    for match in matches:
        if match.endswith('.json'):
            url_json = url + '/' + match
            path_json = os.path.join(save_folder, match)
            
            print('Requesting: ' + url_json)
            req = urllib.request.Request(url_json, headers = headers)
            page = urllib.request.urlopen(req).read()
            
            with open(path_json, 'wb') as f:
                f.write(page)

download_type = sys.argv[1]

if download_type == 'skindata':
    download_jsons_from('https://raw.communitydragon.org/latest/plugins/rcp-be-lol-game-data/global/default/v1/champions/', 'champ_skin_infos')

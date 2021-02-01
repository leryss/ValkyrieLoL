import sys, os, json

def extract_from_folder(folder, save_as):
    result = {}
    for file in os.listdir(folder):
        d = None
        with open(os.path.join(folder, file), 'rb') as f:
            d = json.loads(f.read())
        
        skins = []
        for skininfo in d['skins']:
            chromas = []
            if 'chromas' in skininfo:
                for chroma in skininfo['chromas']:
                    chromas.append({'id': chroma['id'] % 100, 'color': int(chroma['colors'][0][1:], 16)})
                    
            skins.append({
                "name": skininfo["name"],
                "id": skininfo['id'] % 100,
                "chromas": chromas
           }) 
        
        result[d['alias'].lower()] = skins
        
    with open(save_as, 'w') as f:
        f.write(json.dumps(result, indent=4))
    
mode = sys.argv[1]
if mode == 'skininfo':
    extract_from_folder('champ_skin_infos', 'SkinInfo.json')
from pathlib import Path
import hashlib, sys, json, re

release = Path(sys.argv[1]).resolve()
generated = Path(sys.argv[2]).resolve()
release_version = sys.argv[3] if len(sys.argv) > 3 else 'v0.1'
assert re.fullmatch(r'v\d+\.\d+(?:\.\d+)?', release_version), 'Invalid release version'
version_parts = release_version[1:].split('.')
version_parts += ['0'] * (4 - len(version_parts))
version_numbers = ','.join(version_parts)
version_string = '.'.join(version_parts)
generated.mkdir(parents=True, exist_ok=True)
files = [release/'autofarmnongtrai-update.exe', release/'opencv_world4120.dll']
files += sorted(p for folder in ('images', 'licenses') for p in (release/folder).rglob('*') if p.is_file())
digest = hashlib.sha256()
entries, rc = [], ['#include <windows.h>']
for index, path in enumerate(files, 101):
    relative = path.relative_to(release).as_posix()
    digest.update(relative.encode('utf-8') + b'\0' + path.read_bytes())
    entries.append(f'    {{{index}, L{json.dumps(relative)}}},')
    rc.append(f'{index} RCDATA {json.dumps(path.as_posix())}')
version = release_version + '-' + digest.hexdigest()[:16]
header = '#pragma once\nstruct PayloadEntry { unsigned short id; const wchar_t* path; };\n'
header += f'inline constexpr wchar_t kPayloadVersion[] = L"{version}";\n'
header += f'inline constexpr wchar_t kPayloadMutex[] = L"Local\\\\BotFarmerXDLee-{version}";\n'
header += 'inline constexpr PayloadEntry kPayload[] = {\n' + '\n'.join(entries) + '\n};\n'
(generated/'payload.h').write_text(header, encoding='utf-8')
rc.append('''1 VERSIONINFO
FILEVERSION 0,1,0,0
PRODUCTVERSION 0,1,0,0
FILEOS 0x40004
FILETYPE 0x1
BEGIN
 BLOCK "StringFileInfo"
 BEGIN
  BLOCK "040904b0"
  BEGIN
   VALUE "FileDescription", "BOT FARMER X DLee v0.1"
   VALUE "FileVersion", "0.1.0.0"
   VALUE "ProductName", "BOT FARMER X DLee"
   VALUE "ProductVersion", "0.1.0.0"
  END
 END
 BLOCK "VarFileInfo"
 BEGIN
  VALUE "Translation", 0x409, 1200
 END
END''')
resource_text = '\n'.join(rc).replace('0,1,0,0', version_numbers).replace('"0.1.0.0"', f'"{version_string}"')
(generated/'payload.rc').write_text(resource_text, encoding='utf-8')
print(f'Embedded payload: {len(files)} files, cache {version}')

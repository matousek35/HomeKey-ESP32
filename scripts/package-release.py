#!/usr/bin/env python3
"""Package only artifacts produced by this workflow run; fail on missing inputs."""
import hashlib
import re
import shutil
import zipfile
from pathlib import Path

version = Path('VERSION').read_text().strip()
if not re.fullmatch(r'loxone-v\d+\.\d+\.\d+', version):
    raise SystemExit('Invalid release version')
if not Path(f'docs/releases/{version}.md').is_file():
    raise SystemExit('Missing release notes')
out = Path('release-dist')
out.mkdir(exist_ok=False)
files = ['merged-binary.bin', 'HomeKey-ESP32.bin', 'spiffs.bin', 'flasher_args.json', 'sdkconfig']
for variant in ['doorbell', 'camera', 'wifi-test']:
    name = f'waveshare-{variant}'
    source = Path('artifacts') / name
    with zipfile.ZipFile(out / f'{name}.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
        for filename in files:
            path = source / filename
            if not path.is_file() or path.stat().st_size == 0:
                raise SystemExit(f'Missing or empty artifact: {path}')
            archive.write(path, filename)
            if filename.endswith('.bin'):
                shutil.copyfile(path, out / f'{name}-{filename}')
        archive.write('docs/fork/INSTALL-CS.md', 'INSTALL-CS.md')
        archive.writestr('BUILD.txt', f'Version: {version}\nCommit: ' + __import__('os').environ.get('GITHUB_SHA', 'local') + '\n')
with zipfile.ZipFile(out / 'documentation.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
    for path in [Path('README.md'), Path('CHANGELOG.md'), Path('LICENSE'), *Path('docs/fork').glob('*.md'), Path(f'docs/releases/{version}.md')]:
        archive.write(path, str(path))
checksums = [f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n' for p in sorted(out.iterdir())]
(out / 'SHA256SUMS').write_text(''.join(checksums))

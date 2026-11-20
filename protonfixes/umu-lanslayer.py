"""DOOM 2016"""

# Same fix file but for outside of steam
# Only difference is the hijack part not being in this one

import os
import shutil
import urllib.request
import zipfile

from protonfixes import util


def main() -> None:
    # Needed since GBE is being used
    util.set_environment('PROTON_DISABLE_LSTEAMCLIENT', '1')

    # Install CChromaEditorLibrary
    installpath = os.path.abspath(os.getcwd())
    backup = os.path.join(installpath, 'CChromaEditorLibrary.dll.bak')
    url = 'https://github.com/Riesi/CChromaEditor/files/2277158/CChromaEditorLibrary.zip'

    if not os.path.isfile(backup):
        urllib.request.urlretrieve(url, 'CChromaEditorLibrary.zip')
        shutil.move(
            os.path.join(installpath, 'CChromaEditorLibrary.dll'),
            backup,
        )
        with zipfile.ZipFile('CChromaEditorLibrary.zip', 'r') as zip_ref:
            zip_ref.extractall(installpath)

    # Install MFC runtime
    util.protontricks('mfc140')

"""DOOM 2016"""

# Same fix for non-Steam versions.
# The hijack part is omitted here.
# Sets the CWD to the parent directory since launchers usually set it (On steam its same place as DOOMx64.exe is in)
# to the directory containing launched exe, which would prevent the CChroma DLL from being replaced.

import os
import shutil
import urllib.request
import zipfile

from protonfixes import util


def main() -> None:
    # Needed since GBE is being used
    util.set_environment('PROTON_DISABLE_LSTEAMCLIENT', '1')

    # Install MFC runtime
    util.protontricks('mfc140')

    # Go to the parent directory since the launcher is game's installed to a folder there.
    os.chdir('..')

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

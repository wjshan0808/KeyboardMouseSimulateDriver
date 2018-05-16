
#include "stdafx.h"

#include "NseFileSystem.h"


// This globally scoped filename is only used during SaveAs file dialog phase
// to store a perceived file-system filepath for the virtual file.
WCHAR CNseFileSystem::m_wszOpenSaveAsFilename[MAX_PATH];


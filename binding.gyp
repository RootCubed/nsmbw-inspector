{
    "targets": [
        {
            "target_name": "dr",
            "sources": [
                "DolphinReader/DolphinReader.cpp",
                "DolphinReader/Dolphin-memory-engine/Source/Common/MemoryCommon.cpp",
                "DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/DolphinAccessor.cpp",
                "DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/Windows/WindowsDolphinProcess.cpp"
            ],
            "include_dirs": ["<!(node -e \"require('nan')\")"]
        }
    ]
}
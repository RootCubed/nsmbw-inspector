{
    "targets": [
        {
            "target_name": "dr",
            "conditions": [
                [
                    'OS=="win"', {
                        "sources": ["DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/Windows/WindowsDolphinProcess.cpp"]
                    }
                ],
                [
                    'OS=="linux"', {
                        "sources": ["DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/Linux/LinuxDolphinProcess.cpp"]
                    }
                ]
            ],
            "sources": [
                "DolphinReader/DolphinReader.cpp",
                "DolphinReader/Dolphin-memory-engine/Source/Common/MemoryCommon.cpp",
                "DolphinReader/Dolphin-memory-engine/Source/DolphinProcess/DolphinAccessor.cpp"
            ],
            "cflags": ["-fexceptions"],
            "cflags_cc": ["-fexceptions"],
            "include_dirs": ["<!(node -e \"require('nan')\")"]
        }
    ]
}
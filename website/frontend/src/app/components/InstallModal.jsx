import { X, Download, Monitor, Apple, Cpu } from "lucide-react";
import { useMemo } from "react";

function detectPlatform() {
    const ua = navigator.userAgent.toLowerCase();
    if (ua.includes("win")) return "windows";
    if (ua.includes("mac")) return "mac";
    if (ua.includes("linux") || ua.includes("android")) return "linux";
    return "windows"; // default
}

const RELEASES = {
    windows: {
        label: "Windows",
        icon: Monitor,
        version: "v1.0",
        filename: "Quanta_Installer_v1.0.exe",
        fileSize: "Stable (from Inno Setup)",
        releaseDate: "March 7, 2026",
        downloadUrl: "/downloads/Quanta_Installer_v1.0.exe",
        arch: "x64 / ARM64",
    },
    mac: {
        label: "macOS",
        icon: Apple,
        version: "v1.0.0",
        filename: "Quanta Studio-1.0.0-arm64.dmg",
        fileSize: "Stable (Apple Silicon)",
        releaseDate: "March 7, 2026",
        downloadUrl: "/downloads/Quanta Studio-1.0.0-arm64.dmg",
        arch: "Apple Silicon / Intel",
    },
    linux: {
        label: "Linux",
        icon: Cpu,
        version: "v1.0.0",
        filename: "Quanta Studio-1.0.0.AppImage",
        fileSize: "Stable (x64/ARM)",
        releaseDate: "March 7, 2026",
        downloadUrl: "/downloads/Quanta Studio-1.0.0.AppImage",
        arch: "x64 / ARM64",
    },
};

const OTHER_PLATFORMS = {
    windows: ["mac", "linux"],
    mac: ["windows", "linux"],
    linux: ["windows", "mac"],
};

export function InstallModal({ isOpen, onClose }) {
    const platform = useMemo(() => detectPlatform(), []);

    if (!isOpen) return null;

    const primary = RELEASES[platform];
    const others = OTHER_PLATFORMS[platform].map(p => RELEASES[p]);
    const PrimaryIcon = primary.icon;

    return (
        <>
            {/* Backdrop */}
            <div
                className="fixed inset-0 bg-background/80 backdrop-blur-sm z-50"
                onClick={onClose}
            />

            {/* Modal */}
            <div className="fixed left-1/2 top-1/2 -translate-x-1/2 -translate-y-1/2 w-full max-w-lg z-50">
                <div className="bg-card border border-border rounded-2xl shadow-2xl overflow-hidden">
                    {/* Header */}
                    <div className="flex items-center justify-between p-6 border-b border-border">
                        <div className="flex items-center gap-3">
                            <img src="/logo.jpeg" alt="Quanta" className="w-9 h-9 rounded-lg object-cover" />
                            <div>
                                <h2 className="text-xl font-semibold">Download Quanta Studio</h2>
                                <p className="text-xs text-muted-foreground">v1.0 — Stable Release</p>
                            </div>
                        </div>
                        <button
                            onClick={onClose}
                            className="w-8 h-8 rounded-lg hover:bg-muted flex items-center justify-center transition-colors"
                        >
                            <X className="w-5 h-5" />
                        </button>
                    </div>

                    <div className="p-6 space-y-5">
                        {/* Detected Platform — Primary Download */}
                        <div className="bg-gradient-to-r from-[#22d3ee]/10 to-[#a855f7]/10 border border-[#22d3ee]/25 rounded-xl p-5">
                            <div className="flex items-center gap-2 mb-1 text-xs text-[#22d3ee] font-medium">
                                <PrimaryIcon className="w-3.5 h-3.5" />
                                Detected: {primary.label} — Recommended for you
                            </div>
                            <div className="flex items-center justify-between">
                                <div>
                                    <p className="font-semibold text-lg">{primary.filename}</p>
                                    <p className="text-sm text-muted-foreground mt-0.5">
                                        {primary.fileSize} · {primary.arch} · {primary.releaseDate}
                                    </p>
                                </div>
                                <a
                                    href={primary.downloadUrl}
                                    download
                                    className="flex items-center gap-2 px-5 py-2.5 rounded-xl bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium text-sm hover:shadow-lg hover:shadow-[#22d3ee]/25 hover:scale-105 transition-all"
                                >
                                    <Download className="w-4 h-4" />
                                    Download
                                </a>
                            </div>
                        </div>

                        {/* Other Platforms */}
                        <div>
                            <p className="text-xs font-medium text-muted-foreground mb-3 uppercase tracking-wide">Other Platforms</p>
                            <div className="space-y-2">
                                {others.map((rel) => {
                                    const Icon = rel.icon;
                                    return (
                                        <div key={rel.label} className="flex items-center justify-between px-4 py-3 rounded-xl border border-border hover:border-[#22d3ee]/20 hover:bg-muted/30 transition-all">
                                            <div className="flex items-center gap-3">
                                                <Icon className="w-4 h-4 text-muted-foreground" />
                                                <div>
                                                    <p className="text-sm font-medium">{rel.label}</p>
                                                    <p className="text-xs text-muted-foreground">{rel.fileSize} · {rel.arch}</p>
                                                </div>
                                            </div>
                                            <a
                                                href={rel.downloadUrl}
                                                download
                                                className="flex items-center gap-1.5 px-3 py-1.5 rounded-lg border border-border hover:border-[#22d3ee]/30 text-xs font-medium transition-all"
                                            >
                                                <Download className="w-3 h-3" /> Download
                                            </a>
                                        </div>
                                    );
                                })}
                            </div>
                        </div>

                        <p className="text-xs text-muted-foreground text-center pt-1">
                            All releases are free and open source · <a href="https://github.com" className="text-[#22d3ee] hover:underline">View on GitHub</a>
                        </p>
                    </div>
                </div>
            </div>
        </>
    );
}

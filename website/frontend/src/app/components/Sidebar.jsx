import { Link, useLocation } from "react-router";
import { FileText, Zap, Code, Info, Play } from "lucide-react";

export function Sidebar() {
    const location = useLocation();

    const menuItems = [
        { name: "Introduction", path: "/", icon: FileText },
        { name: "Quickstart Guide", path: "/quickstart", icon: Zap },
        { name: "Core Syntax", path: "/syntax", icon: Code },
        { name: "About Quanta", path: "/about", icon: Info },
        { name: "Try Quanta ⚡", path: "/try", icon: Play },
    ];

    return (
        <aside className="fixed left-0 top-[72px] bottom-0 w-[260px] bg-background border-r border-border overflow-y-auto">
            <div className="p-6">
                <h3 className="text-xs uppercase tracking-wider text-muted-foreground mb-4 px-3">
                    Docs
                </h3>
                <nav className="space-y-1">
                    {menuItems.map((item) => {
                        const isActive = location.pathname === item.path;
                        const Icon = item.icon;
                        return (
                            <Link
                                key={item.path}
                                to={item.path}
                                className={`flex items-center gap-3 px-3 py-2.5 rounded-lg text-sm transition-all ${isActive
                                    ? "bg-gradient-to-r from-[#22d3ee]/10 to-[#a855f7]/10 text-[#22d3ee] border border-[#22d3ee]/20"
                                    : "text-muted-foreground hover:text-foreground hover:bg-card"
                                    }`}
                            >
                                <Icon className="w-4 h-4" />
                                <span>{item.name}</span>
                            </Link>
                        );
                    })}
                </nav>
            </div>
        </aside>
    );
}

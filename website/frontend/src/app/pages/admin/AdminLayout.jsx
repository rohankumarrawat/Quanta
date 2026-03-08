import { useState, useEffect } from "react";
import { NavLink, Outlet, useNavigate } from "react-router";
import { useAuth } from "../../../context/AuthContext";
import {
    LayoutDashboard, Users, MessageSquare, FileText,
    Bell, LogOut, Shield, ChevronRight, Menu, X
} from "lucide-react";

const navItems = [
    { icon: LayoutDashboard, label: "Dashboard", path: "/admin" },
    { icon: Users, label: "Users", path: "/admin/users" },
    { icon: MessageSquare, label: "Questions", path: "/admin/questions" },
    { icon: FileText, label: "Blog Posts", path: "/admin/blog" },
    { icon: Bell, label: "Notifications", path: "/admin/notifications" },
];

export function AdminLayout() {
    const { user, logout } = useAuth();
    const navigate = useNavigate();
    const [sidebarOpen, setSidebarOpen] = useState(true);

    const handleLogout = () => {
        logout();
        navigate("/");
    };

    return (
        <div className="min-h-screen bg-background flex">
            {/* Sidebar */}
            <aside className={`fixed top-0 left-0 h-full z-40 flex flex-col border-r border-border bg-card transition-all duration-300 ${sidebarOpen ? "w-[240px]" : "w-[60px]"}`}>
                {/* Logo Area */}
                <div className="flex items-center justify-between px-4 py-4 border-b border-border">
                    {sidebarOpen && (
                        <div className="flex items-center gap-2">
                            <img src="/logo.jpeg" alt="Quanta" className="w-8 h-8 rounded-lg object-cover" />
                            <span className="font-bold text-sm">Admin Portal</span>
                        </div>
                    )}
                    <button
                        onClick={() => setSidebarOpen(v => !v)}
                        className="p-1.5 rounded-lg hover:bg-muted transition-colors text-muted-foreground"
                    >
                        {sidebarOpen ? <X className="w-4 h-4" /> : <Menu className="w-4 h-4" />}
                    </button>
                </div>

                {/* Nav Items */}
                <nav className="flex-1 py-4 space-y-1 px-2">
                    {navItems.map(({ icon: Icon, label, path }) => (
                        <NavLink
                            key={path}
                            to={path}
                            end={path === "/admin"}
                            className={({ isActive }) =>
                                `flex items-center gap-3 px-3 py-2.5 rounded-lg text-sm font-medium transition-all ${isActive
                                    ? "bg-gradient-to-r from-[#22d3ee]/15 to-[#a855f7]/15 text-[#22d3ee] border border-[#22d3ee]/20"
                                    : "text-muted-foreground hover:text-foreground hover:bg-muted"
                                }`
                            }
                            title={!sidebarOpen ? label : undefined}
                        >
                            <Icon className="w-4 h-4 flex-shrink-0" />
                            {sidebarOpen && <span>{label}</span>}
                        </NavLink>
                    ))}
                </nav>

                {/* User info + logout */}
                <div className="p-3 border-t border-border">
                    {sidebarOpen && (
                        <div className="flex items-center gap-2 px-2 py-2 mb-2">
                            <div className="w-7 h-7 rounded-full bg-gradient-to-br from-[#22d3ee] to-[#a855f7] flex items-center justify-center text-background text-xs font-bold flex-shrink-0">
                                {user?.username?.[0]?.toUpperCase()}
                            </div>
                            <div className="min-w-0">
                                <p className="text-xs font-medium truncate">{user?.username}</p>
                                <p className="text-xs text-muted-foreground truncate">{user?.email}</p>
                            </div>
                        </div>
                    )}
                    <button
                        onClick={handleLogout}
                        className="flex items-center gap-3 w-full px-3 py-2 rounded-lg text-sm text-red-400 hover:bg-red-400/10 transition-colors"
                        title={!sidebarOpen ? "Logout" : undefined}
                    >
                        <LogOut className="w-4 h-4 flex-shrink-0" />
                        {sidebarOpen && <span>Logout</span>}
                    </button>
                </div>
            </aside>

            {/* Main Content */}
            <main className={`flex-1 transition-all duration-300 ${sidebarOpen ? "ml-[240px]" : "ml-[60px]"}`}>
                {/* Top Bar */}
                <div className="sticky top-0 z-30 flex items-center justify-between px-8 py-4 border-b border-border bg-background/80 backdrop-blur-sm">
                    <div className="flex items-center gap-2 text-sm text-muted-foreground">
                        <span className="text-[#22d3ee] font-medium">Quanta Admin</span>
                        <ChevronRight className="w-3 h-3" />
                    </div>
                    <a
                        href="/"
                        className="text-xs text-muted-foreground hover:text-foreground flex items-center gap-1.5 transition-colors"
                    >
                        ← Back to Website
                    </a>
                </div>

                {/* Page Content */}
                <div className="p-8">
                    <Outlet />
                </div>
            </main>
        </div>
    );
}

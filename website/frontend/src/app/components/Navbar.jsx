import { Link, useLocation } from "react-router";
import { Terminal, Shield, Menu, X } from "lucide-react";
import { useState } from "react";
import { InstallModal } from "./InstallModal";
import { UserMenu } from "./UserMenu";
import { LoginModal } from "./LoginModal";
import { RegisterModal } from "./RegisterModal";
import { useAuth } from "../../context/AuthContext";

export function Navbar() {
    const location = useLocation();
    const { user, logout, isAdmin } = useAuth();
    const [isInstallModalOpen, setIsInstallModalOpen] = useState(false);
    const [isLoginModalOpen, setIsLoginModalOpen] = useState(false);
    const [isRegisterModalOpen, setIsRegisterModalOpen] = useState(false);
    const [isMobileMenuOpen, setIsMobileMenuOpen] = useState(false);

    const tabs = [
        { name: "Docs", path: "/" },
        { name: "Blog", path: "/blog" },
        { name: "Community", path: "/community" },
        { name: "Try Quanta ⚡", path: "/try" },
    ];

    const getActiveTab = () => {
        if (location.pathname === "/community") return "/community";
        if (location.pathname === "/blog") return "/blog";
        if (location.pathname === "/try") return "/try";
        if (location.pathname.startsWith("/admin")) return "/admin";
        return "/";
    };

    return (
        <>
            <nav className="fixed top-0 left-0 right-0 z-50 bg-background/80 backdrop-blur-xl border-b border-border">
                <div className="h-[72px] px-4 md:px-8 flex items-center justify-between max-w-[1440px] mx-auto">
                    {/* Logo */}
                    <Link to="/" className="flex items-center gap-3 group">
                        <img
                            src="/logo.jpeg"
                            alt="Quanta"
                            className="w-10 h-10 rounded-lg object-cover transition-transform group-hover:scale-105"
                        />
                        <span className="text-xl font-semibold bg-gradient-to-r from-[#22d3ee] to-[#a855f7] bg-clip-text text-transparent">
                            Quanta
                        </span>
                    </Link>

                    {/* Desktop Center Tabs */}
                    <div className="hidden md:flex items-center gap-8">
                        {tabs.map((tab) => {
                            const isActive = getActiveTab() === tab.path;
                            return (
                                <Link
                                    key={tab.path}
                                    to={tab.path}
                                    className={`relative py-2 text-sm transition-colors group ${isActive ? "text-foreground" : "text-muted-foreground hover:text-foreground"
                                        }`}
                                >
                                    {tab.name}
                                    <span className={`absolute bottom-0 left-0 right-0 h-[2px] bg-gradient-to-r from-[#22d3ee] to-[#a855f7] transition-transform origin-left ${isActive ? "scale-x-100" : "scale-x-0 group-hover:scale-x-100"
                                        }`} />
                                </Link>
                            );
                        })}
                    </div>

                    {/* Desktop Right Section */}
                    <div className="hidden md:flex items-center gap-4">
                        {isAdmin && (
                            <Link
                                to="/admin"
                                className="flex items-center gap-1.5 px-3 py-2 rounded-lg bg-[#a855f7]/10 text-[#a855f7] border border-[#a855f7]/20 text-sm font-medium hover:bg-[#a855f7]/20 transition-all"
                            >
                                <Shield className="w-3.5 h-3.5" />
                                Admin
                            </Link>
                        )}
                        <button
                            onClick={() => setIsInstallModalOpen(true)}
                            className="px-6 py-2.5 rounded-full bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background text-sm font-medium hover:shadow-lg hover:shadow-[#22d3ee]/25 transition-all hover:scale-105"
                        >
                            Install v1.0
                        </button>
                        <UserMenu
                            onLoginClick={() => setIsLoginModalOpen(true)}
                            onRegisterClick={() => setIsRegisterModalOpen(true)}
                            onLogout={logout}
                        />
                    </div>

                    {/* Mobile Menu Toggle */}
                    <div className="flex md:hidden items-center gap-4">
                        <UserMenu
                            onLoginClick={() => setIsLoginModalOpen(true)}
                            onRegisterClick={() => setIsRegisterModalOpen(true)}
                            onLogout={logout}
                        />
                        <button
                            onClick={() => setIsMobileMenuOpen(!isMobileMenuOpen)}
                            className="p-2 -mr-2 text-foreground/80 hover:text-foreground transition-colors"
                        >
                            {isMobileMenuOpen ? <X className="w-6 h-6" /> : <Menu className="w-6 h-6" />}
                        </button>
                    </div>
                </div>

                {/* Mobile Menu Overlay */}
                {isMobileMenuOpen && (
                    <div className="md:hidden border-t border-border bg-background/95 backdrop-blur-xl absolute top-[72px] left-0 right-0 shadow-2xl">
                        <div className="flex flex-col p-4 space-y-4">
                            {tabs.map((tab) => {
                                const isActive = getActiveTab() === tab.path;
                                return (
                                    <Link
                                        key={tab.path}
                                        to={tab.path}
                                        onClick={() => setIsMobileMenuOpen(false)}
                                        className={`px-4 py-3 rounded-xl text-base font-medium transition-colors ${isActive ? "bg-muted text-foreground" : "text-muted-foreground hover:bg-muted/50 hover:text-foreground"
                                            }`}
                                    >
                                        {tab.name}
                                    </Link>
                                );
                            })}

                            <div className="pt-4 border-t border-border flex flex-col gap-3 px-4">
                                {isAdmin && (
                                    <Link
                                        to="/admin"
                                        onClick={() => setIsMobileMenuOpen(false)}
                                        className="flex items-center justify-center gap-2 w-full py-3 rounded-xl bg-[#a855f7]/10 text-[#a855f7] border border-[#a855f7]/20 font-medium hover:bg-[#a855f7]/20 transition-all text-base"
                                    >
                                        <Shield className="w-4 h-4" />
                                        Admin Dashboard
                                    </Link>
                                )}
                                <button
                                    onClick={() => {
                                        setIsMobileMenuOpen(false);
                                        setIsInstallModalOpen(true);
                                    }}
                                    className="w-full py-3 rounded-xl bg-gradient-to-r from-[#22d3ee] to-[#a855f7] text-background font-medium hover:shadow-lg hover:shadow-[#22d3ee]/25 transition-all text-base"
                                >
                                    Install Quanta v1.0
                                </button>
                            </div>
                        </div>
                    </div>
                )}
            </nav>

            <InstallModal
                isOpen={isInstallModalOpen}
                onClose={() => setIsInstallModalOpen(false)}
            />
            <LoginModal
                isOpen={isLoginModalOpen}
                onClose={() => setIsLoginModalOpen(false)}
                onSwitchToRegister={() => { setIsLoginModalOpen(false); setIsRegisterModalOpen(true); }}
            />
            <RegisterModal
                isOpen={isRegisterModalOpen}
                onClose={() => setIsRegisterModalOpen(false)}
                onSwitchToLogin={() => { setIsRegisterModalOpen(false); setIsLoginModalOpen(true); }}
            />
        </>
    );
}

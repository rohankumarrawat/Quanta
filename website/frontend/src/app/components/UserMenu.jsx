import { useState, useRef, useEffect } from "react";
import { Link } from "react-router";
import { User, Settings, LogOut } from "lucide-react";
import { useAuth } from "../../context/AuthContext";

export function UserMenu({ onLoginClick, onRegisterClick, onLogout }) {
    const { user } = useAuth();
    const [isDropdownOpen, setIsDropdownOpen] = useState(false);
    const dropdownRef = useRef(null);

    useEffect(() => {
        function handleClickOutside(event) {
            if (dropdownRef.current && !dropdownRef.current.contains(event.target)) {
                setIsDropdownOpen(false);
            }
        }
        document.addEventListener("mousedown", handleClickOutside);
        return () => document.removeEventListener("mousedown", handleClickOutside);
    }, []);

    if (!user) {
        return (
            <div className="flex items-center gap-3">
                <button
                    onClick={onLoginClick}
                    className="px-4 py-2 rounded-lg border border-border text-sm font-medium hover:border-[#22d3ee]/50 hover:text-[#22d3ee] transition-all"
                >
                    Login
                </button>
                <button
                    onClick={onRegisterClick}
                    className="px-4 py-2 rounded-lg border border-[#22d3ee]/50 text-[#22d3ee] text-sm font-medium hover:bg-[#22d3ee]/10 transition-all"
                >
                    Register
                </button>
            </div>
        );
    }

    const initials = user.username
        ? user.username.slice(0, 2).toUpperCase()
        : "QD";

    return (
        <div className="relative" ref={dropdownRef}>
            <button
                onClick={() => setIsDropdownOpen(!isDropdownOpen)}
                className="w-10 h-10 rounded-full bg-gradient-to-br from-[#22d3ee] to-[#a855f7] flex items-center justify-center hover:shadow-lg hover:shadow-[#22d3ee]/25 transition-all hover:scale-105"
            >
                <span className="text-sm font-semibold text-background">{initials}</span>
            </button>

            {isDropdownOpen && (
                <div className="absolute right-0 top-12 w-52 bg-card border border-border rounded-xl shadow-2xl overflow-hidden z-50">
                    <div className="px-4 py-3 border-b border-border">
                        <p className="text-sm font-medium text-foreground">{user.username}</p>
                        <p className="text-xs text-muted-foreground truncate">{user.email}</p>
                    </div>
                    <Link to="/profile" onClick={() => setIsDropdownOpen(false)} className="w-full px-4 py-3 text-left hover:bg-muted transition-colors flex items-center gap-3 text-sm">
                        <User className="w-4 h-4 text-muted-foreground" />
                        <span>Profile</span>
                    </Link>
                    <Link to="/settings" onClick={() => setIsDropdownOpen(false)} className="w-full px-4 py-3 text-left hover:bg-muted transition-colors flex items-center gap-3 text-sm">
                        <Settings className="w-4 h-4 text-muted-foreground" />
                        <span>Settings</span>
                    </Link>
                    <div className="border-t border-border" />
                    <button
                        onClick={() => { onLogout(); setIsDropdownOpen(false); }}
                        className="w-full px-4 py-3 text-left hover:bg-muted transition-colors flex items-center gap-3 text-sm text-red-400"
                    >
                        <LogOut className="w-4 h-4" />
                        <span>Logout</span>
                    </button>
                </div>
            )}
        </div>
    );
}

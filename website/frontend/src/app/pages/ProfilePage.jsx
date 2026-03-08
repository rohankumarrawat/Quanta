import { useAuth } from "../../context/AuthContext";
import { User, Mail, Shield, Calendar } from "lucide-react";

export function ProfilePage() {
    const { user } = useAuth();

    if (!user) {
        return (
            <div className="flex items-center justify-center min-h-[50vh]">
                <p className="text-muted-foreground">Please log in to view your profile.</p>
            </div>
        );
    }

    return (
        <div className="max-w-3xl mx-auto space-y-8">
            <div className="space-y-4">
                <h1 className="text-4xl font-bold">Your Profile</h1>
                <p className="text-muted-foreground">
                    Manage your Quanta account details and view your activity.
                </p>
            </div>

            <div className="bg-card border border-border rounded-xl p-8 space-y-8 relative overflow-hidden">
                {/* Decorative background element */}
                <div className="absolute top-0 right-0 w-64 h-64 bg-gradient-to-br from-[#22d3ee]/5 to-[#a855f7]/5 rounded-full blur-3xl -mr-32 -mt-32 pointer-events-none" />

                <div className="flex items-center gap-6">
                    <div className="w-24 h-24 rounded-full bg-gradient-to-br from-[#22d3ee] to-[#a855f7] flex items-center justify-center text-3xl font-bold text-background shadow-lg">
                        {user.username ? user.username.slice(0, 2).toUpperCase() : "Q"}
                    </div>
                    <div>
                        <h2 className="text-2xl font-bold">{user.username}</h2>
                        <div className="flex items-center gap-2 mt-2">
                            <span className="px-2.5 py-0.5 rounded-full bg-[#22d3ee]/10 text-[#22d3ee] text-xs font-semibold capitalize border border-[#22d3ee]/20">
                                {user.role || "Member"}
                            </span>
                        </div>
                    </div>
                </div>

                <div className="grid grid-cols-1 md:grid-cols-2 gap-6 pt-6 border-t border-border">
                    <div className="space-y-2">
                        <label className="text-sm font-medium text-muted-foreground flex items-center gap-2">
                            <User className="w-4 h-4" /> Username
                        </label>
                        <p className="text-foreground font-medium p-3 bg-muted/50 rounded-lg border border-border/50">
                            {user.username}
                        </p>
                    </div>

                    <div className="space-y-2">
                        <label className="text-sm font-medium text-muted-foreground flex items-center gap-2">
                            <Mail className="w-4 h-4" /> Email Address
                        </label>
                        <p className="text-foreground font-medium p-3 bg-muted/50 rounded-lg border border-border/50">
                            {user.email}
                        </p>
                    </div>

                    <div className="space-y-2">
                        <label className="text-sm font-medium text-muted-foreground flex items-center gap-2">
                            <Shield className="w-4 h-4" /> Account ID
                        </label>
                        <p className="text-foreground font-mono text-sm p-3 bg-muted/50 rounded-lg border border-border/50 truncate">
                            {user.id}
                        </p>
                    </div>

                    <div className="space-y-2">
                        <label className="text-sm font-medium text-muted-foreground flex items-center gap-2">
                            <Calendar className="w-4 h-4" /> Member Since
                        </label>
                        <p className="text-foreground font-medium p-3 bg-muted/50 rounded-lg border border-border/50">
                            {new Date().toLocaleDateString('en-US', { month: 'long', year: 'numeric' })}
                        </p>
                    </div>
                </div>
            </div>

            <div className="bg-gradient-to-r from-[#22d3ee]/10 to-[#a855f7]/10 border border-[#22d3ee]/20 rounded-xl p-6 text-center">
                <p className="text-muted-foreground text-sm">
                    Profile editing will be available in a future update. For account changes, please contact support.
                </p>
            </div>
        </div>
    );
}

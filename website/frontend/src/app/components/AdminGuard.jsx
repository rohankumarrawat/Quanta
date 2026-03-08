import { Navigate } from "react-router";
import { useAuth } from "../../context/AuthContext";

export function AdminGuard({ children }) {
    const { user, isLoading, isAdmin } = useAuth();

    if (isLoading) {
        return (
            <div className="min-h-screen bg-background flex items-center justify-center">
                <div className="text-muted-foreground">Loading...</div>
            </div>
        );
    }

    if (!user || !isAdmin) {
        return <Navigate to="/" replace />;
    }

    return children;
}

import { createContext, useContext, useState, useEffect } from "react";
import { loginUser, registerUser } from "../api/auth";

const AuthContext = createContext(null);

export function AuthProvider({ children }) {
    const [user, setUser] = useState(null);
    const [token, setToken] = useState(null);
    const [isLoading, setIsLoading] = useState(true);

    // Load stored auth on mount
    useEffect(() => {
        const storedToken = localStorage.getItem("quanta_token");
        const storedUser = localStorage.getItem("quanta_user");
        if (storedToken && storedUser) {
            setToken(storedToken);
            setUser(JSON.parse(storedUser));
        }
        setIsLoading(false);
    }, []);

    const login = async (email, password) => {
        const data = await loginUser(email, password);
        setToken(data.token);
        setUser(data.user);
        localStorage.setItem("quanta_token", data.token);
        localStorage.setItem("quanta_user", JSON.stringify(data.user));
        return data;
    };

    const register = async (username, email, password) => {
        const data = await registerUser(username, email, password);
        setToken(data.token);
        setUser(data.user);
        localStorage.setItem("quanta_token", data.token);
        localStorage.setItem("quanta_user", JSON.stringify(data.user));
        return data;
    };

    const logout = () => {
        setToken(null);
        setUser(null);
        localStorage.removeItem("quanta_token");
        localStorage.removeItem("quanta_user");
    };

    const isAdmin = user?.role === "admin";
    const isBlocked = user?.isBlocked === true;

    return (
        <AuthContext.Provider value={{ user, token, login, register, logout, isLoading, isAdmin, isBlocked }}>
            {children}
        </AuthContext.Provider>
    );
}

export function useAuth() {
    const ctx = useContext(AuthContext);
    if (!ctx) throw new Error("useAuth must be used within AuthProvider");
    return ctx;
}

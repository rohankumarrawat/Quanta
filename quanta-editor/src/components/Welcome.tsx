import React from 'react';

// Common SVG icons matching VS Code
const IconNewFile = () => (
    <svg width="16" height="16" viewBox="0 0 16 16" fill="var(--blue)">
        <path d="M14 4.5V14a1 1 0 0 1-1 1H3a1 1 0 0 1-1-1V2a1 1 0 0 1 1-1h6.5L14 4.5zM9.5 1h-.5v4h4v-.5L9.5 1z" />
    </svg>
);

const IconOpenFolder = () => (
    <svg width="16" height="16" viewBox="0 0 16 16" fill="var(--blue)">
        <path d="M14.5 4H7.51l-1-1H2.5A1.5 1.5 0 0 0 1 4v8a1.5 1.5 0 0 0 1.5 1.5h11A1.5 1.5 0 0 0 15 12V5.5A1.5 1.5 0 0 0 13.5 4H14.5zM2 4h4.59l1 1H13.5a.5.5 0 0 1 .5.5v7a.5.5 0 0 1-.5.5h-11a.5.5 0 0 1-.5-.5V4z" />
    </svg>
);

const IconCloneRepo = () => (
    <svg width="16" height="16" viewBox="0 0 16 16" fill="var(--blue)">
        <path d="M11.5 12a1.5 1.5 0 1 0 0-3 1.5 1.5 0 0 0 0 3zM4.5 7A1.5 1.5 0 1 0 4.5 4 1.5 1.5 0 0 0 4.5 7zM11.5 7a1.5 1.5 0 1 0 0-3 1.5 1.5 0 0 0 0 3zm-6 3a1.5 1.5 0 1 0 0-3 1.5 1.5 0 0 0 0 3z" />
        <path d="M5.525 5.5A2.49 2.49 0 0 1 8 7.5c1.196 0 2.222-.841 2.475-2M5.525 10.5A2.49 2.49 0 0 0 8 8.5" fill="none" stroke="var(--blue)" strokeWidth="1" />
    </svg>
);

const IconWalkthrough = () => (
    <svg width="16" height="16" viewBox="0 0 16 16" fill="var(--blue)">
        <path d="M8 1a7 7 0 1 0 0 14A7 7 0 0 0 8 1zm0 13A6 6 0 1 1 8 2a6 6 0 0 1 0 12zM7.5 4h1v4.5A.5.5 0 0 1 8 9a.5.5 0 0 1-.5-.5V4zm1 5.5h-1v1h1v-1z" />
    </svg>
);

// Types
export interface RecentItem {
    name: string;
    path: string;
    type: 'folder' | 'file';
    timestamp: number;
}

interface WelcomeProps {
    onNewFile: () => void;
    onOpenFolder: () => void;
    recentItems: RecentItem[];
    onOpenRecent: (item: RecentItem) => void;
}

export const Welcome: React.FC<WelcomeProps> = ({
    onNewFile,
    onOpenFolder,
    recentItems,
    onOpenRecent
}) => {
    return (
        <div className="welcome-container">
            <div className="welcome-content">
                <div className="welcome-header">
                    <h1>Quanta Studio</h1>
                    <p className="welcome-subtitle">Editing evolved</p>
                </div>

                <div className="welcome-columns">
                    {/* LEFT COLUMN */}
                    <div className="welcome-left">
                        <section className="welcome-section">
                            <h2>Start</h2>
                            <ul className="welcome-list">
                                <li>
                                    <button className="welcome-action" onClick={onNewFile}>
                                        <IconNewFile />
                                        <span>New File...</span>
                                    </button>
                                </li>
                                <li>
                                    <button className="welcome-action" onClick={onOpenFolder}>
                                        <IconOpenFolder />
                                        <span>Open Folder...</span>
                                    </button>
                                </li>
                                <li>
                                    <button className="welcome-action">
                                        <IconCloneRepo />
                                        <span>Clone Git Repository...</span>
                                    </button>
                                </li>
                            </ul>
                        </section>

                        <section className="welcome-section" style={{ marginTop: '24px' }}>
                            <h2>Recent</h2>
                            {recentItems.length === 0 ? (
                                <p className="welcome-empty">No recent folders opened.</p>
                            ) : (
                                <ul className="welcome-list recent-list">
                                    {recentItems.slice(0, 5).map((item, idx) => (
                                        <li key={idx}>
                                            <button className="welcome-recent" onClick={() => onOpenRecent(item)}>
                                                <span className="recent-name">{item.name}</span>
                                                <span className="recent-path">{item.path}</span>
                                            </button>
                                        </li>
                                    ))}
                                    {recentItems.length > 5 && (
                                        <li>
                                            <button className="welcome-action more-link">More...</button>
                                        </li>
                                    )}
                                </ul>
                            )}
                        </section>
                    </div>

                    {/* RIGHT COLUMN */}
                    <div className="welcome-right">
                        <section className="welcome-section">
                            <h2>Walkthroughs</h2>
                            <ul className="welcome-list walkthrough-list">
                                <li>
                                    <button className="welcome-walkthrough">
                                        <IconWalkthrough />
                                        <div className="walkthrough-info">
                                            <span>Learn the Fundamentals</span>
                                            <div className="walkthrough-progress"><div className="progress-bar empty"></div></div>
                                        </div>
                                    </button>
                                </li>
                                <li>
                                    <button className="welcome-walkthrough">
                                        <IconWalkthrough />
                                        <div className="walkthrough-info">
                                            <span>Get Started with Quanta Development <span className="badge-blue">Updated</span></span>
                                            <div className="walkthrough-progress"><div className="progress-bar empty"></div></div>
                                        </div>
                                    </button>
                                </li>
                                <li>
                                    <button className="welcome-walkthrough">
                                        <IconWalkthrough />
                                        <div className="walkthrough-info">
                                            <span>Build a Windows Installer</span>
                                            <div className="walkthrough-progress"><div className="progress-bar half"></div></div>
                                        </div>
                                    </button>
                                </li>
                            </ul>
                        </section>
                    </div>
                </div>
            </div>
        </div>
    );
};

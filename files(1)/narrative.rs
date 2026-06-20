use crate::emotion::{self, Emotion};
use crate::git::{Author, Commit, RepoStats};

pub struct Story {
    pub title:      String,
    pub chapters:   Vec<Chapter>,
    pub characters: Vec<CharacterProfile>,
    pub verdict:    String,
}

pub struct Chapter {
    pub heading: String,
    pub body:    String,
    pub era:     (usize, usize), // commit index range
}

pub struct CharacterProfile {
    pub name:    String,
    pub archetype: &'static str,
    pub summary: String,
    pub badge:   &'static str,
}

// ─── main entry ──────────────────────────────────────────────────────────────

pub fn generate(stats: &RepoStats) -> Story {
    let commits  = &stats.commits;
    let emotions: Vec<Emotion> = commits.iter().map(emotion::detect).collect();
    let authors  = &stats.authors;

    let title    = derive_title(stats, &emotions);
    let chapters = derive_chapters(commits, &emotions, stats);
    let characters = derive_characters(authors, commits, &emotions);
    let verdict  = derive_verdict(stats, &emotions);

    Story { title, chapters, characters, verdict }
}

// ─── title ───────────────────────────────────────────────────────────────────

fn derive_title(stats: &RepoStats, emotions: &[Emotion]) -> String {
    let panic_count   = emotions.iter().filter(|&&e| e == Emotion::Panic).count();
    let triumph_count = emotions.iter().filter(|&&e| e == Emotion::Triumph).count();
    let despair_count = emotions.iter().filter(|&&e| e == Emotion::Despair).count();
    let total         = emotions.len().max(1);
    let primary_author = stats.authors.first().map(|a| a.name.as_str()).unwrap_or("Someone");

    if panic_count as f32 / total as f32 > 0.2 {
        format!("{}: A Story Told in Hotfixes", stats.name)
    } else if triumph_count as f32 / total as f32 > 0.15 {
        format!("{}: Against All Odds, It Shipped", stats.name)
    } else if despair_count as f32 / total as f32 > 0.2 {
        format!("{}: {} and the Bugs That Wouldn't Die", stats.name, primary_author)
    } else if stats.total_commits > 500 {
        format!("{}: An Epic in {} Acts", stats.name, stats.total_commits / 100)
    } else {
        format!("{}: The Untold Story", stats.name)
    }
}

// ─── chapters ────────────────────────────────────────────────────────────────

fn derive_chapters(commits: &[Commit], emotions: &[Emotion], stats: &RepoStats) -> Vec<Chapter> {
    if commits.is_empty() { return vec![]; }

    let n = commits.len();
    let ranges = vec![(0, n/3), (n/3, 2*n/3), (2*n/3, n)];
    let labels = ["The Beginning", "The Struggle", "The Reckoning"];

    ranges.into_iter().enumerate().filter_map(|(i, (start, end))| {
        if start >= end { return None; }

        let era_commits  = &commits[start..end];
        let era_emotions = &emotions[start..end];

        let body = narrate_era(era_commits, era_emotions, i, stats);
        Some(Chapter {
            heading: labels[i].to_string(),
            body,
            era: (start, end),
        })
    }).collect()
}

fn narrate_era(commits: &[Commit], emotions: &[Emotion], era: usize, stats: &RepoStats) -> String {
    let n = commits.len();
    if n == 0 { return String::new(); }

    let first   = &commits[0];
    let last    = &commits[n - 1];
    let days    = ((last.date_ts - first.date_ts) / 86400).abs();

    let panic_n   = emotions.iter().filter(|&&e| e == Emotion::Panic).count();
    let triumph_n = emotions.iter().filter(|&&e| e == Emotion::Triumph).count();
    let despair_n = emotions.iter().filter(|&&e| e == Emotion::Despair).count();
    let struggle_n= emotions.iter().filter(|&&e| e == Emotion::Struggle).count();
    let late_n    = commits.iter().filter(|c| c.hour < 4 || c.hour >= 23).count();
    let wkend_n   = commits.iter().filter(|c| c.weekday == 0 || c.weekday == 6).count();

    // pick a real commit message to quote as flavor
    let quote = emotions.iter().enumerate()
        .find(|(_, &e)| e == match era {
            0 => Emotion::Ambition,
            1 => Emotion::Struggle,
            _ => Emotion::Triumph,
        })
        .and_then(|(i, _)| commits.get(i))
        .map(|c| c.subject.as_str())
        .unwrap_or(&first.subject);

    let author = &first.author;

    match era {
        // ── Beginning
        0 => {
            let energy = if n > 20 { "relentless pace" } else { "careful, deliberate pace" };
            let side   = if wkend_n > n / 3 { "even weekends offered no rest" }
                         else { "weekdays were enough, back then" };
            format!(
                "It started with \"{}\". {} committed {} times across {} days, at a {}, and {}. \
                 There was no hesitation in the early messages — only momentum. \
                 The kind of momentum that comes before anyone has told you it won't work.",
                quote, author, n, days, energy, side
            )
        }

        // ── Middle
        1 => {
            let crisis = if panic_n > 0 {
                format!("{} of those commits were marked as critical or urgent — \
                         the kind of messages written when sleep-deprived fingers move faster than thought.", panic_n)
            } else if struggle_n > n / 3 {
                format!("{} commits were spent fighting bugs — \
                         the same problems wearing different disguises.", struggle_n)
            } else {
                "the commits grew quieter, more deliberate, as if the project knew what it was becoming.".into()
            };

            let night = if late_n > 3 {
                format!("{} commits arrived between midnight and 4am. The terminal glows differently at 3am.", late_n)
            } else {
                "The work stayed mostly in daylight hours. Mostly.".into()
            };

            format!(
                "Then came the middle, which is where all honest software stories live. \
                 \"{}\". {}. {}. \
                 This was the phase where enthusiasm becomes endurance.",
                quote, crisis, night
            )
        }

        // ── End
        _ => {
            let tone = if triumph_n > despair_n {
                "The recent commits carry a different weight — \
                 fewer apologies, more declarative statements. \
                 Something was figured out."
            } else if despair_n > triumph_n {
                "The most recent messages are quieter but heavier. \
                 The kind of quiet that comes not from peace but from exhaustion."
            } else {
                "The project reached a kind of equilibrium — \
                 not finished, not broken, just ongoing."
            };

            let hot = stats.hottest_files.first()
                .map(|(f, n)| format!("{} was rewritten {} times and never truly finished", f, n))
                .unwrap_or_else(|| "some files carry more history than others".into());

            format!(
                "The latest chapter is still being written. \"{}\". {}. And {}. \
                 These are the commits of someone who knows the codebase now — \
                 for better and worse.",
                quote, tone, hot
            )
        }

        _ => String::new(),
    }
}

// ─── character profiles ───────────────────────────────────────────────────────

fn derive_characters(authors: &[Author], commits: &[Commit], emotions: &[Emotion]) -> Vec<CharacterProfile> {
    authors.iter().take(4).map(|a| {
        let archetype = classify_archetype(a);
        let summary   = write_profile(a, commits, emotions, archetype);
        let badge     = archetype_badge(archetype);
        CharacterProfile {
            name: a.name.clone(),
            archetype,
            summary,
            badge,
        }
    }).collect()
}

fn classify_archetype(a: &Author) -> &'static str {
    let late_ratio  = a.late_commits as f32 / a.commits.max(1) as f32;
    let fix_ratio   = a.fix_msgs as f32    / a.commits.max(1) as f32;
    let panic_ratio = a.panic_msgs as f32  / a.commits.max(1) as f32;

    if panic_ratio > 0.15 { return "The Firefighter"; }
    if late_ratio  > 0.30 { return "The Night Owl";   }
    if fix_ratio   > 0.40 { return "The Debugger";    }

    let days_active = (a.last_ts - a.first_ts) / 86400;
    if days_active > 0 {
        let commits_per_day = a.commits as f64 / days_active as f64;
        if commits_per_day > 5.0 { return "The Sprinter"; }
    }

    if a.longest_gap > 86400 * 30 { return "The Ghost"; }

    "The Architect"
}

fn archetype_badge(archetype: &str) -> &'static str {
    match archetype {
        "The Firefighter" => "🔥",
        "The Night Owl"   => "🦉",
        "The Debugger"    => "🐛",
        "The Sprinter"    => "⚡",
        "The Ghost"       => "👻",
        _                 => "🏛",
    }
}

fn write_profile(a: &Author, _commits: &[Commit], _emotions: &[Emotion], archetype: &str) -> String {
    let days = ((a.last_ts - a.first_ts) / 86400).max(1);
    let gap_days = a.longest_gap / 86400;

    match archetype {
        "The Firefighter" => format!(
            "{} commits, {} of them marked urgent, critical, or reverting someone else's work. \
             {} doesn't write features — they extinguish consequences. \
             The codebase is safer for it, and they are probably more tired.",
            a.commits, a.panic_msgs, a.name
        ),
        "The Night Owl" => format!(
            "{} commits, {} of them filed between midnight and 4am. \
             {} found their rhythm in the dark, when Slack is silent and the compiler is the only one listening. \
             The quality of that judgment remains an open question.",
            a.commits, a.late_commits, a.name
        ),
        "The Debugger" => format!(
            "{} commits, {} of them fixes, patches, or corrections. \
             {} is the one who inherits the consequences of other people's optimism. \
             Without them, half the repository would be a polite lie.",
            a.commits, a.fix_msgs, a.name
        ),
        "The Sprinter" => format!(
            "{} commits across {} days — a velocity that is either inspiring or alarming. \
             {} codes in bursts, each one carrying the urgency of a deadline that may or may not exist. \
             The git log keeps up. Just.",
            a.commits, days, a.name
        ),
        "The Ghost" => format!(
            "{} commits, and then a {} day silence. \
             {} appears, leaves a mark, and disappears. \
             Some of their code is still running. They may not know this.",
            a.commits, gap_days, a.name
        ),
        _ => format!(
            "{} commits across {} days. \
             {} is the constant in this equation — \
             not the most dramatic presence, but the one the project could least afford to lose.",
            a.commits, days, a.name
        ),
    }
}

// ─── verdict ─────────────────────────────────────────────────────────────────

fn derive_verdict(stats: &RepoStats, emotions: &[Emotion]) -> String {
    let n             = emotions.len().max(1);
    let panic_ratio   = emotions.iter().filter(|&&e| e == Emotion::Panic).count() as f32   / n as f32;
    let triumph_ratio = emotions.iter().filter(|&&e| e == Emotion::Triumph).count() as f32 / n as f32;
    let despair_ratio = emotions.iter().filter(|&&e| e == Emotion::Despair).count() as f32 / n as f32;
    let total_days    = ((stats.last_ts - stats.first_ts) / 86400).max(1);
    let velocity      = stats.total_commits as f32 / total_days as f32;

    let health = if panic_ratio > 0.2 {
        "a codebase in permanent crisis mode"
    } else if triumph_ratio > 0.15 {
        "a codebase that has found its rhythm"
    } else if despair_ratio > 0.2 {
        "a codebase carrying more weight than its commit messages admit"
    } else if velocity > 3.0 {
        "a codebase built by someone who cannot stop"
    } else {
        "a codebase that has survived its own ambitions"
    };

    let line_note = if let Some((f, _)) = stats.hottest_files.first() {
        format!(" The file '{}' has been rewritten so many times it has become a palimpsest — \
                 each version haunted by the one before.", f)
    } else {
        String::new()
    };

    format!(
        "{} commits. {} contributors. {} days. This is {}. {}",
        stats.total_commits,
        stats.authors.len(),
        total_days,
        health,
        line_note,
    )
}

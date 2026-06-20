use std::collections::HashMap;
use std::path::Path;
use std::process::Command;

#[derive(Debug, Clone)]
pub struct Commit {
    pub sha:     String,
    pub author:  String,
    pub date_ts: i64,   // unix timestamp
    pub hour:    u32,
    pub weekday: u32,   // 0=Sun 6=Sat
    pub subject: String,
    pub insertions: usize,
    pub deletions:  usize,
}

#[derive(Debug, Clone)]
pub struct Author {
    pub name:         String,
    pub commits:      usize,
    pub late_commits: usize,   // midnight–4am
    pub panic_msgs:   usize,
    pub fix_msgs:     usize,
    pub longest_gap:  u64,     // seconds between their commits
    pub first_ts:     i64,
    pub last_ts:      i64,
}

#[derive(Debug)]
pub struct RepoStats {
    pub name:          String,
    pub total_commits: usize,
    pub authors:       Vec<Author>,
    pub hottest_files: Vec<(String, usize)>,
    pub commits:       Vec<Commit>,
    pub first_ts:      i64,
    pub last_ts:       i64,
}

// ─── helpers ─────────────────────────────────────────────────────────────────

fn git(args: &[&str], cwd: &Path) -> Option<String> {
    Command::new("git")
        .args(args)
        .current_dir(cwd)
        .output()
        .ok()
        .filter(|o| o.status.success())
        .map(|o| String::from_utf8_lossy(&o.stdout).trim().to_string())
}

pub fn is_git_repo(path: &Path) -> bool {
    git(&["rev-parse", "--git-dir"], path).is_some()
}

pub fn collect(path: &Path, limit: usize) -> RepoStats {
    let name = git(&["remote", "get-url", "origin"], path)
        .map(|u| u.trim_end_matches('/').split('/').last()
               .unwrap_or("repo").trim_end_matches(".git").to_string())
        .unwrap_or_else(|| path.file_name()
               .and_then(|n| n.to_str()).unwrap_or("repo").to_string());

    // format: SHA|author|unix_ts|hour|weekday|subject
    let fmt    = "--pretty=format:%H|%an|%ct|%H|%s";
    let lim    = format!("-{}", limit);
    let raw    = git(&["log", fmt, &lim], path).unwrap_or_default();

    // also get numstat for insertions/deletions
    let stat_raw = git(
        &["log", "--numstat", "--pretty=format:COMMIT:%H", &lim],
        path,
    ).unwrap_or_default();

    // parse numstat into map: sha -> (ins, del)
    let mut stat_map: HashMap<String, (usize, usize)> = HashMap::new();
    {
        let mut cur_sha = String::new();
        for line in stat_raw.lines() {
            if let Some(sha) = line.strip_prefix("COMMIT:") {
                cur_sha = sha.trim()[..8.min(sha.trim().len())].to_string();
            } else {
                let parts: Vec<&str> = line.split('\t').collect();
                if parts.len() >= 2 {
                    let ins = parts[0].parse::<usize>().unwrap_or(0);
                    let del = parts[1].parse::<usize>().unwrap_or(0);
                    let e   = stat_map.entry(cur_sha.clone()).or_default();
                    e.0 += ins;
                    e.1 += del;
                }
            }
        }
    }

    let mut commits: Vec<Commit> = Vec::new();

    for line in raw.lines() {
        let parts: Vec<&str> = line.splitn(5, '|').collect();
        if parts.len() < 5 { continue; }

        let sha     = parts[0][..8.min(parts[0].len())].to_string();
        let author  = parts[1].to_string();
        let ts: i64 = parts[2].parse().unwrap_or(0);
        let subject = parts[4].to_string();

        // derive hour and weekday from timestamp
        // days since epoch
        let secs_day = 86400i64;
        // unix epoch was a Thursday (weekday 4)
        let days  = ts / secs_day;
        let weekday = ((days + 4) % 7) as u32;
        let hour_of_day = ((ts % secs_day) / 3600) as u32;

        let (ins, del) = stat_map.get(&sha).copied().unwrap_or((0, 0));

        commits.push(Commit {
            sha,
            author,
            date_ts: ts,
            hour: hour_of_day,
            weekday,
            subject,
            insertions: ins,
            deletions:  del,
        });
    }

    // sort oldest→newest for timeline
    commits.sort_by_key(|c| c.date_ts);

    let first_ts = commits.first().map(|c| c.date_ts).unwrap_or(0);
    let last_ts  = commits.last().map(|c| c.date_ts).unwrap_or(0);

    // build author stats
    let mut author_map: HashMap<String, Vec<&Commit>> = HashMap::new();
    for c in &commits {
        author_map.entry(c.author.clone()).or_default().push(c);
    }

    let panic_words  = ["hotfix","urgent","critical","revert","emergency","broken","disaster"];
    let fix_words    = ["fix","bug","patch","repair","correct","issue","error","mistake","wrong"];

    let mut authors: Vec<Author> = author_map.iter().map(|(name, cs)| {
        let late   = cs.iter().filter(|c| c.hour < 4 || c.hour >= 23).count();
        let panic  = cs.iter().filter(|c| panic_words.iter()
                                      .any(|w| c.subject.to_lowercase().contains(w))).count();
        let fix    = cs.iter().filter(|c| fix_words.iter()
                                      .any(|w| c.subject.to_lowercase().contains(w))).count();

        let mut sorted_ts: Vec<i64> = cs.iter().map(|c| c.date_ts).collect();
        sorted_ts.sort();
        let longest_gap = sorted_ts.windows(2)
            .map(|w| (w[1] - w[0]) as u64)
            .max()
            .unwrap_or(0);

        Author {
            name:         name.clone(),
            commits:      cs.len(),
            late_commits: late,
            panic_msgs:   panic,
            fix_msgs:     fix,
            longest_gap,
            first_ts:     sorted_ts.first().copied().unwrap_or(0),
            last_ts:      sorted_ts.last().copied().unwrap_or(0),
        }
    }).collect();

    authors.sort_by(|a, b| b.commits.cmp(&a.commits));

    // hottest files
    let file_raw = git(
        &["log", "--pretty=format:", "--name-only", "--diff-filter=M", &lim],
        path,
    ).unwrap_or_default();

    let mut file_counts: HashMap<String, usize> = HashMap::new();
    for line in file_raw.lines() {
        let f = line.trim();
        if !f.is_empty() {
            *file_counts.entry(f.to_string()).or_default() += 1;
        }
    }
    let mut hottest: Vec<(String, usize)> = file_counts.into_iter().collect();
    hottest.sort_by(|a, b| b.1.cmp(&a.1));
    hottest.truncate(5);

    let total_commits = git(&["rev-list", "--count", "HEAD"], path)
        .and_then(|s| s.parse().ok())
        .unwrap_or(commits.len());

    RepoStats { name, total_commits, authors, hottest_files: hottest, commits, first_ts, last_ts }
}

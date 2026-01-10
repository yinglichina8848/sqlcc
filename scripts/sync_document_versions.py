#!/usr/bin/env python3
"""
SQLCC Documentation Version Synchronization Script

This script synchronizes version information across all documentation files
to ensure consistency between code versions and documentation versions.

Usage:
    python3 scripts/sync_document_versions.py [--dry-run] [--current-version VERSION]

Options:
    --dry-run           Show what would be changed without making changes
    --current-version   Specify the current version (default: read from VERSION file)

Author: SQLCC Documentation Team
Last Updated: 2026-01-11
"""

import os
import re
import argparse
from pathlib import Path
from typing import Dict, List, Tuple

class DocVersionSync:
    def __init__(self, current_version: str):
        self.current_version = current_version
        self.docs_dir = Path("docs")
        self.changes_made = []
        self.errors = []

    def get_current_version(self) -> str:
        """Get current version from VERSION file"""
        version_file = Path("VERSION")
        if version_file.exists():
            return version_file.read_text().strip()
        return self.current_version

    def find_doc_files(self) -> List[Path]:
        """Find all documentation files that might contain version information"""
        doc_files = []

        # Common documentation files
        patterns = [
            "*.md",
            "**/*.md"
        ]

        for pattern in patterns:
            doc_files.extend(self.docs_dir.glob(pattern))

        return sorted(doc_files)

    def extract_version_info(self, file_path: Path) -> Dict[str, str]:
        """Extract version information from a documentation file"""
        try:
            content = file_path.read_text(encoding='utf-8')
            version_info = {}

            # Look for version patterns
            version_patterns = [
                r'## 📦 Current Version: (v?\d+\.\d+\.\d+)',
                r'\* 版本: (v?\d+\.\d+\.\d+)',
                r'Version: (v?\d+\.\d+\.\d+)',
            ]

            for pattern in version_patterns:
                match = re.search(pattern, content, re.IGNORECASE)
                if match:
                    version_info['current_version'] = match.group(1)
                    break

            # Look for last updated patterns
            update_patterns = [
                r'\* 最后更新时间: (\d{4}-\d{2}-\d{2})',
                r'Last Updated: (\d{4}-\d{2}-\d{2})',
                r'最后更新时间: (\d{4}-\d{2}-\d{2})',
            ]

            for pattern in update_patterns:
                match = re.search(pattern, content)
                if match:
                    version_info['last_updated'] = match.group(1)
                    break

            return version_info

        except Exception as e:
            self.errors.append(f"Error reading {file_path}: {e}")
            return {}

    def update_version_info(self, file_path: Path, current_version: str, today_date: str) -> bool:
        """Update version information in a documentation file"""
        try:
            content = file_path.read_text(encoding='utf-8')
            original_content = content

            # Update current version
            content = re.sub(
                r'(## 📦 Current Version:) (v?\d+\.\d+\.\d+)',
                f'\\1 {current_version}',
                content
            )

            # Update version in metadata
            content = re.sub(
                r'(\* 版本:) (v?\d+\.\d+\.\d+)',
                f'\\1 {current_version}',
                content
            )

            # Update last updated date
            content = re.sub(
                r'(\* 最后更新时间:) (\d{4}-\d{2}-\d{2})',
                f'\\1 {today_date}',
                content
            )

            content = re.sub(
                r'(Last Updated:) (\d{4}-\d{2}-\d{2})',
                f'\\1 {today_date}',
                content
            )

            if content != original_content:
                file_path.write_text(content, encoding='utf-8')
                self.changes_made.append(f"Updated {file_path}")
                return True

            return False

        except Exception as e:
            self.errors.append(f"Error updating {file_path}: {e}")
            return False

    def sync_versions(self, dry_run: bool = False) -> Tuple[int, int]:
        """Synchronize version information across all documentation files"""
        from datetime import datetime
        today = datetime.now().strftime('%Y-%m-%d')
        current_version = self.get_current_version()

        print(f"Current version: {current_version}")
        print(f"Today's date: {today}")
        print(f"Dry run: {dry_run}")
        print("-" * 50)

        doc_files = self.find_doc_files()
        updated_count = 0
        checked_count = 0

        for file_path in doc_files:
            checked_count += 1
            version_info = self.extract_version_info(file_path)

            needs_update = (
                version_info.get('current_version') != current_version or
                version_info.get('last_updated') != today
            )

            if needs_update:
                if dry_run:
                    print(f"Would update: {file_path}")
                    print(f"  Current: {version_info.get('current_version', 'N/A')}")
                    print(f"  Target:  {current_version}")
                else:
                    if self.update_version_info(file_path, current_version, today):
                        updated_count += 1
                        print(f"Updated: {file_path}")
                    else:
                        print(f"No changes needed: {file_path}")
            else:
                if dry_run:
                    print(f"Already up to date: {file_path}")

        return updated_count, checked_count

    def report_results(self, updated_count: int, checked_count: int):
        """Report the synchronization results"""
        print("\n" + "=" * 50)
        print("SYNCHRONIZATION COMPLETE")
        print("=" * 50)
        print(f"Files checked: {checked_count}")
        print(f"Files updated: {updated_count}")

        if self.changes_made:
            print("\nChanges made:")
            for change in self.changes_made:
                print(f"  ✓ {change}")

        if self.errors:
            print("\nErrors encountered:")
            for error in self.errors:
                print(f"  ✗ {error}")

def main():
    parser = argparse.ArgumentParser(description='Synchronize documentation versions')
    parser.add_argument('--dry-run', action='store_true',
                       help='Show what would be changed without making changes')
    parser.add_argument('--current-version', type=str,
                       help='Specify the current version')

    args = parser.parse_args()

    # Get current version
    current_version = args.current_version
    if not current_version:
        version_file = Path("VERSION")
        if version_file.exists():
            current_version = version_file.read_text().strip()
        else:
            print("Error: Could not determine current version. Use --current-version or ensure VERSION file exists.")
            return 1

    # Create synchronizer
    sync = DocVersionSync(current_version)

    # Perform synchronization
    updated_count, checked_count = sync.sync_versions(args.dry_run)

    # Report results
    sync.report_results(updated_count, checked_count)

    return 0

if __name__ == "__main__":
    exit(main())

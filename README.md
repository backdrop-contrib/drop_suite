# Drop Suite

A productivity and CRM platform for [Backdrop CMS](https://backdropcms.org).

Drop Suite ships as a single contrib project containing a parent module and a curated set of submodules for managing contacts, projects, documentation, and team file libraries. The parent module is enabled first and provides the shared chrome (navigation, CSS variables, project entity, team directory, settings dashboard); each submodule is enabled independently for the features you want.

## What's in this project

| Module | Description | Path | Dependencies |
|---|---|---|---|
| **Drop Suite** | Parent module. Shared CSS, project entity, primary nav, settings dashboard, team directory, meeting agenda. Required by everything else. | `drop/`, `drop/team`, `drop/settings`, `drop/projects`, `drop/meeting` | entity_plus |
| **Drop Dashboard** | Daily landing page at `/drop` — a unified to-do list aggregating items from other Drop modules via `module_exists()` checks. | `drop` | drop_suite |
| **Drop Start** | Sample content generation and onboarding wizard for fresh installs. | — | drop_suite |
| **Drop CRM** | Lightweight CRM with contacts (people, organizations, households), activities, and contact methods. Full Views integration. | `drop/crm` | drop_suite, entity_plus, entity_ui, entityreference |
| **Drop CRM User** *(submodule)* | Bridges Backdrop user accounts to Drop CRM contacts. Optional auto-create on user registration. | — | drop_crm |
| **Drop CRM Membership** *(submodule, deprecated)* | Older membership tracking built into Drop CRM. Kept for now but no longer developed: use Drop Membership Entity for new sites. | `drop/crm/memberships` | drop_crm |
| **Drop Docs** | Hierarchical team wiki/documentation built on Backdrop's Book module. | `drop/docs` | drop_suite, book |
| **Drop Files** | Team file sharing library with tag-based organization. | `drop/files` | drop_suite, taxonomy |
| **Drop Pay** | Payments for Drop Suite modules: a payment record, online checkout through a gateway (Stripe included), offline payments (cash, check) and refunds. Other modules start payments and react when one completes. | `drop/pay/settings/payments` | entity_plus |
| **Drop Pay Stripe** *(submodule)* | Stripe Checkout gateway for Drop Pay. | `drop/pay/settings/gateway/stripe` | drop_pay, stripe |
| **Drop Membership Entity** | Memberships for Drop Suite, built on the contrib Membership Entity module: Drop Suite pages and navigation, a price for each term length, and online join and renewal payments through Drop Pay (optional). | `drop/memberships` | drop_suite, membership_entity, membership_entity_term |

The project also ships the **Drop Suite layout** (bundled inside the parent module) — a full-width application layout used for `/drop/*` pages.

## Architecture at a glance

- **Drop Suite** (the parent) is the foundation. Every submodule depends on it. It owns the project entity, primary navigation, layout, settings dashboard framework, and admin-shortcut buttons (Add Person, Add Board).
- **Drop Dashboard** aggregates content from other Drop modules into a daily list. Other modules don't depend on it; it depends on them via `module_exists()` so it gracefully skips features that aren't enabled.
- **Drop CRM** is the data hub. Contacts, activities, and contact methods live here. Submodules add user-account bridging (and the deprecated Drop CRM Membership).
- **Drop Membership Entity** handles memberships. Membership Entity stores the memberships and terms; this module adds the Drop Suite pages, prices and payments. With Drop CRM User enabled, payments are linked to the member's CRM contact.
- **Drop Docs** and **Drop Files** are independent content modules — they don't talk to Drop CRM, just share the chrome and conventions provided by the parent.

## Requirements

- Backdrop CMS 1.x
- [Entity Plus](https://github.com/backdrop-contrib/entity_plus) — required by Drop Suite (parent) and Drop CRM
- [Entity UI](https://github.com/backdrop-contrib/entity_ui) — required by Drop CRM
- [Entity Reference](https://github.com/backdrop-contrib/entityreference) — required by Drop CRM
- [Membership Entity](https://github.com/backdrop-contrib/membership_entity) 1.x-1.2.3 or later — required by Drop Membership Entity
- [Stripe](https://github.com/backdrop-contrib/stripe) — required by Drop Pay Stripe

Backdrop core's **Book** module is required by Drop Docs (already in core, just needs enabling). **Taxonomy** is required by Drop Files (also core).

## Permissions

Drop Suite ships a **Team Member** role on fresh install with day-to-day perms (no admin-tier capabilities). The parent module also adds a `team_member` role config so new sites get a sensible default for non-administrator users.

Permission name conventions:

- `access <module>` — view and use basic features
- `administer <module>` — full administrative access (`restrict access`)
- Module-specific verbs (e.g. `create crm contacts`, `view drop docs book content`)

A scoped permissions page is available at `admin/drop/permissions` showing only Drop Suite modules' permissions for cleaner role configuration.

## Configuration

Settings live at `drop/settings` — a dashboard page that lists configuration entry points for every enabled Drop Suite module. Each module's setting page is accessible there and via direct paths:

- **Drop Suite**: `drop/settings/base` (page widths, meeting feature toggle)
- **Drop CRM**: `drop/crm/settings` (activity types, contact-method labels, relationship types, outreach goals)
- **Drop Docs**: `drop/docs/settings`
- **Drop Files**: `drop/files/settings`
- **Drop Membership Entity**: `drop/memberships/settings` (leads to the membership type settings, where term lengths and prices are set)

Most configuration is stored in Backdrop's config system (JSON files in `config/active/`).

---

## Notes About Use of AI

This module was developed with significant assistance from AI tools (specifically Claude by Anthropic). AI was used to generate code, plan features, and make iterative improvements throughout development.

We encourage folks to review the code and help us identify quirky things that AI might have done or places where it may not be following Backdrop CMS best practices. Pull requests and issue reports are welcome.

---

## Installation

Install this module using the official Backdrop CMS instructions at https://backdropcms.org/guide/modules

---

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.

---

## Current Maintainers

- Tim Erickson ([@stpaultim](https://github.com/stpaultim))

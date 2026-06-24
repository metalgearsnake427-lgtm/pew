import { Link } from "wouter";

export function Footer() {
  return (
    <footer className="bg-foreground text-background py-16 px-4 md:px-8">
      <div className="max-w-7xl mx-auto grid grid-cols-1 md:grid-cols-4 gap-12">
        <div className="md:col-span-2">
          <Link href="/">
            <span className="text-3xl font-serif cursor-pointer hover:text-primary transition-colors">Form & Void</span>
          </Link>
          <p className="mt-4 text-background/60 max-w-sm font-light">
            An award-winning architecture and home planning studio crafting eternal spaces for modern living.
          </p>
        </div>
        
        <div>
          <h4 className="uppercase tracking-widest text-xs font-semibold mb-6 text-primary">Navigation</h4>
          <ul className="space-y-4 text-background/70">
            <li><a href="#projects" className="hover:text-primary transition-colors">Selected Works</a></li>
            <li><a href="#services" className="hover:text-primary transition-colors">Expertise</a></li>
            <li><a href="#contact" className="hover:text-primary transition-colors">Contact</a></li>
          </ul>
        </div>
        
        <div>
          <h4 className="uppercase tracking-widest text-xs font-semibold mb-6 text-primary">Socials</h4>
          <ul className="space-y-4 text-background/70">
            <li><a href="#" className="hover:text-primary transition-colors">Instagram</a></li>
            <li><a href="#" className="hover:text-primary transition-colors">Pinterest</a></li>
            <li><a href="#" className="hover:text-primary transition-colors">LinkedIn</a></li>
          </ul>
        </div>
      </div>
      
      <div className="max-w-7xl mx-auto mt-16 pt-8 border-t border-background/10 flex flex-col md:flex-row justify-between items-center gap-4 text-xs text-background/40">
        <p>&copy; {new Date().getFullYear()} Form & Void Studio. All rights reserved.</p>
        <div className="flex gap-6">
          <a href="#" className="hover:text-primary transition-colors">Privacy Policy</a>
          <a href="#" className="hover:text-primary transition-colors">Terms of Service</a>
        </div>
      </div>
    </footer>
  );
}

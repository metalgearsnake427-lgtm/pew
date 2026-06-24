import { useRoute, Link } from "wouter";
import { useGetProject } from "@workspace/api-client-react";
import { motion } from "framer-motion";
import { Skeleton } from "@/components/ui/skeleton";
import { ArrowLeft } from "lucide-react";
import { Footer } from "@/components/sections/Footer";

const IMAGES = [
  "/projects/project1.png",
  "/projects/project2.png",
  "/projects/project3.png",
  "/projects/project4.png",
  "/projects/project5.png",
];

export default function ProjectDetail() {
  const [, params] = useRoute("/project/:id");
  const projectId = params?.id ? parseInt(params.id, 10) : 0;
  
  const { data: project, isLoading } = useGetProject(projectId, {
    query: { enabled: !!projectId }
  });

  if (isLoading) {
    return (
      <div className="min-h-screen w-full bg-background pt-24 px-4 md:px-8">
        <Skeleton className="w-full aspect-[21/9] max-w-7xl mx-auto" />
      </div>
    );
  }

  if (!project) {
    return (
      <div className="min-h-screen flex items-center justify-center bg-background text-foreground flex-col gap-6">
        <h1 className="text-4xl font-serif">Project not found</h1>
        <Link href="/" className="text-primary hover:underline">Return to Home</Link>
      </div>
    );
  }

  const imageSrc = IMAGES[project.id % IMAGES.length];

  return (
    <main className="w-full min-h-screen bg-background text-foreground">
      {/* Header Bar */}
      <header className="fixed top-0 left-0 w-full p-6 z-50 mix-blend-difference text-white">
        <Link href="/" className="inline-flex items-center gap-2 hover:opacity-70 transition-opacity uppercase tracking-widest text-xs font-semibold">
          <ArrowLeft size={16} /> Back to Studio
        </Link>
      </header>

      {/* Hero Image */}
      <div className="relative w-full h-[70vh] md:h-[85vh] overflow-hidden">
        <motion.div 
          initial={{ scale: 1.1 }}
          animate={{ scale: 1 }}
          transition={{ duration: 1.5, ease: "easeOut" }}
          className="w-full h-full"
        >
          <div className="absolute inset-0 bg-black/20 z-10" />
          <img src={imageSrc} alt={project.title} className="w-full h-full object-cover" />
        </motion.div>
        <div className="absolute bottom-10 left-0 w-full px-4 md:px-8 z-20">
          <div className="max-w-7xl mx-auto">
            <motion.h1 
              initial={{ opacity: 0, y: 30 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 1, delay: 0.3 }}
              className="text-5xl md:text-8xl font-serif text-white max-w-4xl"
            >
              {project.title}
            </motion.h1>
          </div>
        </div>
      </div>

      {/* Content */}
      <section className="py-24 px-4 md:px-8">
        <div className="max-w-7xl mx-auto">
          <div className="grid grid-cols-1 md:grid-cols-12 gap-16">
            {/* Meta */}
            <motion.div 
              initial={{ opacity: 0, y: 20 }}
              whileInView={{ opacity: 1, y: 0 }}
              viewport={{ once: true }}
              className="md:col-span-4 space-y-8"
            >
              <div>
                <h4 className="uppercase tracking-widest text-xs text-muted-foreground font-semibold mb-2">Category</h4>
                <p className="text-lg capitalize font-serif">{project.category}</p>
              </div>
              
              {project.client && (
                <div>
                  <h4 className="uppercase tracking-widest text-xs text-muted-foreground font-semibold mb-2">Client</h4>
                  <p className="text-lg font-serif">{project.client}</p>
                </div>
              )}

              {project.location && (
                <div>
                  <h4 className="uppercase tracking-widest text-xs text-muted-foreground font-semibold mb-2">Location</h4>
                  <p className="text-lg font-serif">{project.location}</p>
                </div>
              )}
              
              <div>
                <h4 className="uppercase tracking-widest text-xs text-muted-foreground font-semibold mb-2">Completion</h4>
                <p className="text-lg font-serif">{project.year}</p>
              </div>

              {project.area && (
                <div>
                  <h4 className="uppercase tracking-widest text-xs text-muted-foreground font-semibold mb-2">Total Area</h4>
                  <p className="text-lg font-serif">{project.area}</p>
                </div>
              )}
            </motion.div>

            {/* Description */}
            <motion.div 
              initial={{ opacity: 0, y: 20 }}
              whileInView={{ opacity: 1, y: 0 }}
              viewport={{ once: true }}
              className="md:col-span-8"
            >
              <h3 className="text-3xl font-serif mb-8 text-primary">The Vision</h3>
              <p className="text-xl md:text-2xl font-light leading-relaxed text-muted-foreground">
                {project.description}
              </p>
              
              {project.tags && project.tags.length > 0 && (
                <div className="mt-12 flex flex-wrap gap-3">
                  {project.tags.map((tag) => (
                    <span key={tag} className="px-4 py-2 border border-border text-xs uppercase tracking-widest rounded-none">
                      {tag}
                    </span>
                  ))}
                </div>
              )}
            </motion.div>
          </div>
        </div>
      </section>

      {/* Full width extra image presentation just using the same to give depth */}
      <section className="pb-32 px-4 md:px-8">
        <div className="max-w-7xl mx-auto">
          <motion.div 
            initial={{ opacity: 0, y: 50 }}
            whileInView={{ opacity: 1, y: 0 }}
            viewport={{ once: true }}
            className="aspect-[16/9] w-full relative overflow-hidden"
          >
             <img src={imageSrc} alt={`${project.title} detail`} className="w-full h-full object-cover opacity-80 mix-blend-luminosity hover:scale-105 transition-transform duration-[2s]" />
          </motion.div>
        </div>
      </section>

      <Footer />
    </main>
  );
}
